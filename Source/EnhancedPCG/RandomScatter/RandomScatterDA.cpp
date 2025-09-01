#include "RandomScatterDA.h"
#include "SMDataAsset.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "GameFramework/Volume.h"
#include "Kismet/GameplayStatics.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

ARandomScatterDA::ARandomScatterDA()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 设置默认配置
    MainBuildingLayer.Num = 5;
    MainBuildingLayer.MinDistance = 500.0f;
    MainBuildingLayer.CollisionRadius = 200.0f;
    MainBuildingLayer.bAlignToGround = true;
    MainBuildingLayer.HeightOffsetRange = FVector2D(0.0f, 0.0f);

    SubBuildingLayer.Num = 15;
    SubBuildingLayer.MinDistance = 150.0f;
    SubBuildingLayer.CollisionRadius = 100.0f;
    SubBuildingLayer.bAlignToGround = false;

    SmallObjectLayer.Num = 30;
    SmallObjectLayer.MinDistance = 50.0f;
    SmallObjectLayer.CollisionRadius = 30.0f;
    SmallObjectLayer.bAlignToGround = true;

    ReadJson();
}

void ARandomScatterDA::BeginPlay()
{
    Super::BeginPlay();
    ReadJson();
    ScatterActors();
}

void ARandomScatterDA::ScatterActors()
{
    ClearPreviousScatter();

    TArray<AActor*> FoundActors;
    const FName VolumeTag(TEXT("ScatterBounds"));
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AVolume::StaticClass(), VolumeTag, FoundActors);

    if (FoundActors.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("没有找到散布边界Volume"));
        return;
    }

    for (AActor* act : FoundActors) {
        ScatterBounds.AddUnique(Cast<AVolume>(act));
    }

    for (AVolume* vol : ScatterBounds) {
        ScatterActorsLayered(vol);
    }
}

void ARandomScatterDA::GetMaterialsFromDataAsset(TArray<UMaterialInterface*>& OutMaterials, EScatterLevel Level) const
{
    if (!ScattersDataAsset.IsValid()) {
        USMDataAsset* LoadedDataAsset = ScattersDataAsset.LoadSynchronous();
        if (!LoadedDataAsset) {
            UE_LOG(LogTemp, Error, TEXT("加载MaterialDataAsset失败"));
            return;
        }
    }

    USMDataAsset* DataAsset = ScattersDataAsset.Get();
    if (!DataAsset) return;

    TArray<TSoftObjectPtr<UMaterialInterface>>* MaterialArray = nullptr;
    switch (Level) {
    case EScatterLevel::MainBuilding:
        MaterialArray = &DataAsset->Materials;
        break;
    case EScatterLevel::SubBuilding:
        MaterialArray = &DataAsset->Materials1;
        break;
    case EScatterLevel::SmallObject:
        MaterialArray = &DataAsset->Materials2;
        break;
    }

    if (MaterialArray && MaterialArray->Num() > 0) {
        for (const TSoftObjectPtr<UMaterialInterface>& MaterialRef : *MaterialArray) {
            UMaterialInterface* Material = MaterialRef.LoadSynchronous();
            if (Material) {
                OutMaterials.Add(Material);
            }
        }
    }
}

void ARandomScatterDA::GetMeshFromDataAsset(TArray<UStaticMesh*>& OutAssets, EScatterLevel Level) const
{
    if (!ScattersDataAsset.IsValid()) {
        USMDataAsset* LoadedDataAsset = ScattersDataAsset.LoadSynchronous();
        if (!LoadedDataAsset) {
            UE_LOG(LogTemp, Error, TEXT("加载MeshDataAsset失败"));
            return;
        }
    }

    USMDataAsset* DataAsset = ScattersDataAsset.Get();
    if (!DataAsset) return;

    TArray<TSoftObjectPtr<UStaticMesh>>* MeshArray = nullptr;
    switch (Level) {
    case EScatterLevel::MainBuilding:
        MeshArray = &DataAsset->Meshes;
        break;
    case EScatterLevel::SubBuilding:
        MeshArray = &DataAsset->Meshes1;
        break;
    case EScatterLevel::SmallObject:
        MeshArray = &DataAsset->Meshes2;
        break;
    }

    if (MeshArray && MeshArray->Num() > 0) {
        for (const TSoftObjectPtr<UStaticMesh>& MeshRef : *MeshArray) {
            UStaticMesh* Mesh = MeshRef.LoadSynchronous();
            if (Mesh) {
                OutAssets.Add(Mesh);
            }
        }
    }
}

void ARandomScatterDA::ReadJson()
{
    FString JsonFilePath = FPaths::GameSourceDir() + TEXT("Blocks/Resources/Scatter.json");

    // 读取每个层级的配置
    MainBuildingLayer.LoadFromJson(JsonFilePath, TEXT("MainBuilding"));
    SubBuildingLayer.LoadFromJson(JsonFilePath, TEXT("SubBuilding"));
    SmallObjectLayer.LoadFromJson(JsonFilePath, TEXT("SmallObject"));
}

void ARandomScatterDA::ScatterActorsLayered(const AVolume* Bounds)
{
    FVector Origin = Bounds->GetBounds().Origin;
    FVector BoxExtent = Bounds->GetBounds().BoxExtent;
    FVector Min = Origin - BoxExtent;
    FVector Max = Origin + BoxExtent;

    ScatterMainBuildings(Min, Max);
    ScatterSubBuildings();
    ScatterSmallObjects(Min, Max);

    UE_LOG(LogTemp, Warning, TEXT("散布完成，总共生成 %d 个实例"), ScatteredInstances.Num());
}

void ARandomScatterDA::ScatterMainBuildings(const FVector& Min, const FVector& Max)
{
    const FLayerScatterConfig& LayerConfig = MainBuildingLayer;

    TArray<UMaterialInterface*> Materials;
    TArray<UStaticMesh*> StaticMeshes;
    GetMaterialsFromDataAsset(Materials, EScatterLevel::MainBuilding);
    GetMeshFromDataAsset(StaticMeshes, EScatterLevel::MainBuilding);

    if (StaticMeshes.Num() == 0 || Materials.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("主建筑层级数据不足"));
        return;
    }

    // 使用多阶段采样生成建筑组团
    TArray<FVector> AllLocations;
    GenerateBuildingClusters(Min, Max, LayerConfig, AllLocations);

    int32 SuccessCount = 0;
    for (const FVector& Location : AllLocations) {
        FVector AdjustedLocation = Location;

        // 主建筑必须进行地面对齐射线检测
        if (LayerConfig.bAlignToGround && !AlignToGround(AdjustedLocation, LayerConfig.GroundTraceDistance)) {
            UE_LOG(LogTemp, Log, TEXT("主建筑无法找到地面，跳过位置 (%.1f, %.1f, %.1f)"), Location.X, Location.Y, Location.Z);
            continue;
        }

        // 移除碰撞检测，允许建筑重叠形成复杂体块
        // if (LayerConfig.bUseCollisionDetection && CheckCollision(AdjustedLocation, LayerConfig.CollisionRadius, EScatterLevel::MainBuilding)) {
        //     continue;
        // }

        UStaticMesh* SelectedMesh = StaticMeshes[FMath::RandRange(0, StaticMeshes.Num() - 1)];
        UMaterialInterface* SelectedMaterial = Materials[FMath::RandRange(0, Materials.Num() - 1)];

        UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateHISMC(SelectedMesh, SelectedMaterial);

        // 使用新函数获取实际的transform信息
        FTransform ActualTransform;
        int32 InstanceIndex = AddInstanceWithTransform(HISMC, AdjustedLocation, LayerConfig, ActualTransform);

        if (InstanceIndex >= 0) {
            FScatteredInstanceInfo InstanceInfo;
            InstanceInfo.InstanceIndex = InstanceIndex;
            InstanceInfo.Location = AdjustedLocation;
            InstanceInfo.Level = EScatterLevel::MainBuilding;
            InstanceInfo.HISMComponent = HISMC;

            // 记录实际的bounds、旋转和scale信息
            InstanceInfo.ActualBounds = GetActualMeshBounds(SelectedMesh, ActualTransform.GetScale3D());
            InstanceInfo.Rotation = ActualTransform.GetRotation().Rotator();
            InstanceInfo.Scale = ActualTransform.GetScale3D();

            ScatteredInstances.MainBuildings.Add(InstanceInfo);
            SuccessCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("主建筑完成：生成 %d 个建筑组团"), SuccessCount);
}

void ARandomScatterDA::ScatterSubBuildings()
{
    const FLayerScatterConfig& LayerConfig = SubBuildingLayer;

    TArray<UMaterialInterface*> Materials;
    TArray<UStaticMesh*> StaticMeshes;
    GetMaterialsFromDataAsset(Materials, EScatterLevel::SubBuilding);
    GetMeshFromDataAsset(StaticMeshes, EScatterLevel::SubBuilding);

    if (StaticMeshes.Num() == 0 || Materials.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("附属建筑层级数据不足"));
        return;
    }

    const TSet<FScatteredInstanceInfo>& MainBuildings = ScatteredInstances.MainBuildings;

    if (MainBuildings.Num() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("没有主建筑可供放置附属构筑物"));
        return;
    }

    int32 TotalSuccessCount = 0;
    int32 MainBuildingIndex = 0;

    // 为每个主建筑生成围绕它的副建筑点位
    for (const FScatteredInstanceInfo& TargetBuilding : MainBuildings) {

        // 根据主建筑实际尺寸计算副建筑数量
        int32 SubBuildingsForThisBuilding = CalculateSubBuildingCount(TargetBuilding, LayerConfig);

        // 根据主建筑尺寸调整搜索半径
        FVector BuildingSize = TargetBuilding.ActualBounds.GetSize();
        float BuildingRadius = FMath::Max(BuildingSize.X, BuildingSize.Y) * 0.5f;
        float SearchRadius = FMath::Max(300.0f, BuildingRadius);
        float MinDistance = FMath::Max(LayerConfig.MinDistance, BuildingRadius * 0.3f);

        // 使用泊松圆盘采样在主建筑周围生成均匀分布的点
        TArray<FVector> CandidateLocations;
        GeneratePoissonPointsAroundBuilding(
            TargetBuilding.Location,
            SearchRadius,
            MinDistance,
            SubBuildingsForThisBuilding * 3, // 生成更多候选点
            CandidateLocations);

        int32 CurrentBuildingSuccessCount = 0;

        // 遍历候选位置并尝试放置副建筑
        for (const FVector& BaseLocation : CandidateLocations) {
            if (CurrentBuildingSuccessCount >= SubBuildingsForThisBuilding) {
                break;
            }

            // 添加高度变化
            FVector CandidateLocation = BaseLocation;
            CandidateLocation.Z += FMath::RandRange(50.0f, 150.0f);

            // 检查碰撞
            if (LayerConfig.bUseCollisionDetection && CheckCollision(CandidateLocation, LayerConfig.CollisionRadius, EScatterLevel::SubBuilding)) {
                continue;
            }

            // 选择随机的网格和材质
            UStaticMesh* SelectedMesh = StaticMeshes[FMath::RandRange(0, StaticMeshes.Num() - 1)];
            UMaterialInterface* SelectedMaterial = Materials[FMath::RandRange(0, Materials.Num() - 1)];

            UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateHISMC(SelectedMesh, SelectedMaterial);

            // 创建对齐旋转的LayerConfig副本
            FLayerScatterConfig AlignedConfig = LayerConfig;
            // 副建筑旋转与主建筑对齐：0°, 90°, 180°, 270°
            float BaseYaw = TargetBuilding.Rotation.Yaw;
            float AlignmentOffsets[] = { 0.0f, 90.0f, 180.0f, 270.0f };
            float SelectedOffset = AlignmentOffsets[FMath::RandRange(0, 3)];
            float AlignedYaw = BaseYaw + SelectedOffset;

            // 重写旋转增量以确保对齐
            AlignedConfig.RotationDelta.Yaw = 0.0f; // 不添加随机偏移

            // 使用新函数获取实际的transform信息
            FTransform ActualTransform;
            int32 InstanceIndex = AddInstanceWithTransform(HISMC, CandidateLocation, AlignedConfig, ActualTransform);

            // 手动调整旋转以对齐主建筑
            if (InstanceIndex >= 0 && HISMC) {
                FTransform AlignedTransform = ActualTransform;
                FRotator AlignedRotation = ActualTransform.GetRotation().Rotator();
                AlignedRotation.Yaw = AlignedYaw;
                AlignedTransform.SetRotation(AlignedRotation.Quaternion());

                // 更新HISMC中的实例transform
                HISMC->UpdateInstanceTransform(InstanceIndex, AlignedTransform, false, false);

                FScatteredInstanceInfo InstanceInfo;
                InstanceInfo.InstanceIndex = InstanceIndex;
                InstanceInfo.Location = CandidateLocation;
                InstanceInfo.Level = EScatterLevel::SubBuilding;
                InstanceInfo.HISMComponent = HISMC;

                // 记录实际的bounds、旋转和scale信息
                InstanceInfo.ActualBounds = GetActualMeshBounds(SelectedMesh, AlignedTransform.GetScale3D());
                InstanceInfo.Rotation = AlignedRotation;
                InstanceInfo.Scale = AlignedTransform.GetScale3D();

                ScatteredInstances.SubBuildings.Add(InstanceInfo);
                CurrentBuildingSuccessCount++;
                TotalSuccessCount++;
            }
        }

        UE_LOG(LogTemp, Log, TEXT("主建筑 %d (尺寸: %.1fx%.1f) 周围生成了 %d 个副建筑"), MainBuildingIndex, BuildingSize.X, BuildingSize.Y, CurrentBuildingSuccessCount);
        MainBuildingIndex++;
    }

    UE_LOG(LogTemp, Warning, TEXT("附属建筑完成：生成 %d 个"), TotalSuccessCount);
}

void ARandomScatterDA::ScatterSmallObjects(const FVector& Min, const FVector& Max)
{
    const FLayerScatterConfig& LayerConfig = SmallObjectLayer;

    TArray<UMaterialInterface*> Materials;
    TArray<UStaticMesh*> StaticMeshes;
    GetMaterialsFromDataAsset(Materials, EScatterLevel::SmallObject);
    GetMeshFromDataAsset(StaticMeshes, EScatterLevel::SmallObject);

    if (StaticMeshes.Num() == 0 || Materials.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("小物体层级数据不足"));
        return;
    }

    int32 AttachedCount = FMath::RoundToInt(LayerConfig.Num * 0.7f);
    int32 RandomCount = LayerConfig.Num - AttachedCount;
    int32 TotalSuccessCount = 0;

    if (AttachedCount > 0) {
        TotalSuccessCount += ScatterAttachedSmallObjects(AttachedCount, StaticMeshes, Materials, LayerConfig);
    }

    if (RandomCount > 0) {
        TotalSuccessCount += ScatterRandomSmallObjects(Min, Max, RandomCount, StaticMeshes, Materials, LayerConfig);
    }

    UE_LOG(LogTemp, Warning, TEXT("小物体完成：生成 %d 个"), TotalSuccessCount);
}

bool ARandomScatterDA::AlignToGround(FVector& InOutLocation, float TraceDistance) const
{
    if (!GetWorld()) return false;

    FVector StartLocation = InOutLocation + FVector(0, 0, TraceDistance * 0.5f);
    FVector EndLocation = InOutLocation - FVector(0, 0, TraceDistance * 0.5f);

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(const_cast<ARandomScatterDA*>(this));

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams);

    if (bHit) {
        InOutLocation.Z = HitResult.Location.Z;
        return true;
    }
    return false;
}

bool ARandomScatterDA::CheckCollision(const FVector& Location, float CollisionRadius, EScatterLevel CurrentLevel) const
{
    // 检查所有层级的实例，不只是当前层级
    TArray<FScatteredInstanceInfo> AllInstances;
    for (const FScatteredInstanceInfo& Instance : ScatteredInstances.MainBuildings) {
        AllInstances.Add(Instance);
    }
    for (const FScatteredInstanceInfo& Instance : ScatteredInstances.SubBuildings) {
        AllInstances.Add(Instance);
    }
    for (const FScatteredInstanceInfo& Instance : ScatteredInstances.SmallObjects) {
        AllInstances.Add(Instance);
    }

    for (const auto& ExistingInstance : AllInstances) {

        float Distance = FVector::Dist(Location, ExistingInstance.Location);

        // 计算实际需要的距离
        float RequiredDistance = CollisionRadius;

        // 如果现有实例有实际bounds信息，使用bounds计算
        if (!ExistingInstance.ActualBounds.IsValid) {
            // 回退到原来的逻辑
            if (CurrentLevel == EScatterLevel::SmallObject) {
                RequiredDistance = FMath::Max(CollisionRadius, 50.0f);
            }
            else if (CurrentLevel == EScatterLevel::SubBuilding && ExistingInstance.Level == EScatterLevel::MainBuilding) {
                RequiredDistance = CollisionRadius * 0.7f;
            }
        }
        else {
            // 使用实际bounds计算所需距离
            FVector ExistingSize = ExistingInstance.ActualBounds.GetSize();
            float ExistingRadius = FMath::Max(ExistingSize.X, ExistingSize.Y) * 0.5f;

            if (CurrentLevel == EScatterLevel::MainBuilding) {
                RequiredDistance = FMath::Max(CollisionRadius, ExistingRadius) + 100.0f; // 主建筑之间保持更大距离
            }
            else if (CurrentLevel == EScatterLevel::SubBuilding && ExistingInstance.Level == EScatterLevel::MainBuilding) {
                RequiredDistance = ExistingRadius * 0.8f + CollisionRadius; // 副建筑与主建筑的距离基于主建筑实际尺寸
            }
            else if (CurrentLevel == EScatterLevel::SmallObject) {
                RequiredDistance = FMath::Max(CollisionRadius, ExistingRadius * 0.5f + 25.0f);
            }
            else {
                RequiredDistance = FMath::Max(CollisionRadius, ExistingRadius) + 50.0f;
            }
        }

        if (Distance < RequiredDistance) {
            return true;
        }
    }
    return false;
}

void ARandomScatterDA::GenerateRandomLocations(const FVector& Min, const FVector& Max, const FLayerScatterConfig& LayerConfig, TArray<FVector>& OutLocations) const
{
    OutLocations.Empty();

    FVector AreaSize = Max - Min;
    float CellSize = FMath::Max(LayerConfig.MinDistance, FMath::Min(AreaSize.X, AreaSize.Y) / FMath::Sqrt(LayerConfig.Num));

    TSet<FIntVector> OccupiedCells;
    int32 MaxAttempts = LayerConfig.Num * 20;

    for (int32 Attempts = 0; Attempts < MaxAttempts && OutLocations.Num() < LayerConfig.Num; Attempts++) {
        FVector CandidateLocation = FVector(
            FMath::RandRange(Min.X, Max.X),
            FMath::RandRange(Min.Y, Max.Y),
            Min.Z);

        FIntVector CellIndex = FIntVector(
            FMath::FloorToInt((CandidateLocation.X - Min.X) / CellSize),
            FMath::FloorToInt((CandidateLocation.Y - Min.Y) / CellSize),
            0);

        if (!OccupiedCells.Contains(CellIndex)) {
            OutLocations.Add(CandidateLocation);
            OccupiedCells.Add(CellIndex);
        }
    }
}

UHierarchicalInstancedStaticMeshComponent* ARandomScatterDA::GetOrCreateHISMC(UStaticMesh* Mesh, UMaterialInterface* Material)
{
    // 查找是否已存在相同Mesh和Material的HISMC
    for (auto& HISMCInfo : HISMComponents) {
        if (HISMCInfo.Mesh == Mesh && HISMCInfo.Material == Material) {
            return HISMCInfo.Component;
        }
    }

    UHierarchicalInstancedStaticMeshComponent* NewHISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(
        this,
        UHierarchicalInstancedStaticMeshComponent::StaticClass(),
        FName(*FString::Printf(TEXT("HISMC_%d"), HISMComponents.Num())));

    NewHISMC->SetStaticMesh(Mesh);
    NewHISMC->SetMaterial(0, Material);
    NewHISMC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    NewHISMC->SetVisibility(true);
    NewHISMC->bRenderCustomDepth = true;

    // 注册组件到世界
    NewHISMC->RegisterComponent();

    FHISMCInfo NewInfo;
    NewInfo.Component = NewHISMC;
    NewInfo.Mesh = Mesh;
    NewInfo.Material = Material;
    HISMComponents.Add(NewInfo);

    return NewHISMC;
}

int32 ARandomScatterDA::AddInstance(UHierarchicalInstancedStaticMeshComponent* HISMC, const FVector& Location, const FLayerScatterConfig& LayerConfig)
{
    FTransform OutTransform;
    return AddInstanceWithTransform(HISMC, Location, LayerConfig, OutTransform);
}

int32 ARandomScatterDA::AddInstanceWithTransform(UHierarchicalInstancedStaticMeshComponent* HISMC, const FVector& Location, const FLayerScatterConfig& LayerConfig, FTransform& OutTransform)
{
    if (!HISMC) return -1;

    // 生成随机变换
    FRotator Rotation = FRotator(0, FMath::RandRange(-LayerConfig.RotationDelta.Yaw, LayerConfig.RotationDelta.Yaw), 0);

    float BaseScale = FMath::RandRange(0.8f, 1.2f);
    FVector Scale = FVector(
        BaseScale * FMath::RandRange(1.0f - LayerConfig.ScalarDelta.X, 1.0f + LayerConfig.ScalarDelta.X),
        BaseScale * FMath::RandRange(1.0f - LayerConfig.ScalarDelta.Y, 1.0f + LayerConfig.ScalarDelta.Y),
        BaseScale * FMath::RandRange(1.0f - LayerConfig.ScalarDelta.Z, 1.0f + LayerConfig.ScalarDelta.Z));

    FTransform InstanceTransform = FTransform(Rotation, Location, Scale);
    OutTransform = InstanceTransform;
    return HISMC->AddInstance(InstanceTransform);
}

void ARandomScatterDA::ClearPreviousScatter()
{
    UE_LOG(LogTemp, Warning, TEXT("清理 %d 个HISMC组件"), HISMComponents.Num());

    // 清理所有HISMC组件
    for (auto& HISMCInfo : HISMComponents) {
        if (HISMCInfo.Component && IsValid(HISMCInfo.Component)) {
            // 先清空所有实例
            HISMCInfo.Component->ClearInstances();
            // 然后销毁组件
            HISMCInfo.Component->DestroyComponent();
        }
    }

    HISMComponents.Empty();
    ScatteredInstances.Empty();
}

int32 ARandomScatterDA::ScatterAttachedSmallObjects(int32 TargetCount, const TArray<UStaticMesh*>& StaticMeshes, const TArray<UMaterialInterface*>& Materials, const FLayerScatterConfig& LayerConfig)
{
    TArray<FScatteredInstanceInfo> AttachableObjects;
    // 收集主建筑和副建筑作为可附着对象
    for (const FScatteredInstanceInfo& Instance : ScatteredInstances.MainBuildings) {
        AttachableObjects.Add(Instance);
    }
    for (const FScatteredInstanceInfo& Instance : ScatteredInstances.SubBuildings) {
        AttachableObjects.Add(Instance);
    }

    if (AttachableObjects.Num() == 0) return 0;

    int32 SuccessCount = 0;
    int32 MaxAttempts = TargetCount * 30;

    for (int32 Attempts = 0; Attempts < MaxAttempts && SuccessCount < TargetCount; Attempts++) {
        const FScatteredInstanceInfo& TargetBuilding = AttachableObjects[FMath::RandRange(0, AttachableObjects.Num() - 1)];

        FVector AttachLocation;
        if (!GenerateAttachedLocation(TargetBuilding, AttachLocation)) {
            continue;
        }

        if (LayerConfig.bUseCollisionDetection && CheckCollision(AttachLocation, LayerConfig.CollisionRadius * 0.5f, EScatterLevel::SmallObject)) {
            continue;
        }

        UStaticMesh* SelectedMesh = StaticMeshes[FMath::RandRange(0, StaticMeshes.Num() - 1)];
        UMaterialInterface* SelectedMaterial = Materials[FMath::RandRange(0, Materials.Num() - 1)];

        UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateHISMC(SelectedMesh, SelectedMaterial);

        // 使用新函数获取实际的transform信息
        FTransform ActualTransform;
        int32 InstanceIndex = AddInstanceWithTransform(HISMC, AttachLocation, LayerConfig, ActualTransform);

        if (InstanceIndex >= 0) {
            FScatteredInstanceInfo InstanceInfo;
            InstanceInfo.InstanceIndex = InstanceIndex;
            InstanceInfo.Location = AttachLocation;
            InstanceInfo.Level = EScatterLevel::SmallObject;
            InstanceInfo.HISMComponent = HISMC;

            // 记录实际的bounds、旋转和scale信息
            InstanceInfo.ActualBounds = GetActualMeshBounds(SelectedMesh, ActualTransform.GetScale3D());
            InstanceInfo.Rotation = ActualTransform.GetRotation().Rotator();
            InstanceInfo.Scale = ActualTransform.GetScale3D();

            ScatteredInstances.SmallObjects.Add(InstanceInfo);
            SuccessCount++;
        }
    }

    return SuccessCount;
}

int32 ARandomScatterDA::ScatterRandomSmallObjects(const FVector& Min, const FVector& Max, int32 TargetCount, const TArray<UStaticMesh*>& StaticMeshes, const TArray<UMaterialInterface*>& Materials, const FLayerScatterConfig& LayerConfig)
{
    TArray<FVector> Locations;
    FLayerScatterConfig TempConfig = LayerConfig;
    TempConfig.Num = TargetCount;
    GenerateRandomLocations(Min, Max, TempConfig, Locations);

    int32 SuccessCount = 0;
    for (int32 i = 0; i < FMath::Min(TargetCount, Locations.Num()); i++) {
        FVector Location = Locations[i];

        if (LayerConfig.bAlignToGround && !AlignToGround(Location, LayerConfig.GroundTraceDistance)) {
            continue;
        }

        if (LayerConfig.bUseCollisionDetection && CheckCollision(Location, LayerConfig.CollisionRadius, EScatterLevel::SmallObject)) {
            continue;
        }

        UStaticMesh* SelectedMesh = StaticMeshes[FMath::RandRange(0, StaticMeshes.Num() - 1)];
        UMaterialInterface* SelectedMaterial = Materials[FMath::RandRange(0, Materials.Num() - 1)];

        UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateHISMC(SelectedMesh, SelectedMaterial);

        // 使用新函数获取实际的transform信息
        FTransform ActualTransform;
        int32 InstanceIndex = AddInstanceWithTransform(HISMC, Location, LayerConfig, ActualTransform);

        if (InstanceIndex >= 0) {
            FScatteredInstanceInfo InstanceInfo;
            InstanceInfo.InstanceIndex = InstanceIndex;
            InstanceInfo.Location = Location;
            InstanceInfo.Level = EScatterLevel::SmallObject;
            InstanceInfo.HISMComponent = HISMC;

            // 记录实际的bounds、旋转和scale信息
            InstanceInfo.ActualBounds = GetActualMeshBounds(SelectedMesh, ActualTransform.GetScale3D());
            InstanceInfo.Rotation = ActualTransform.GetRotation().Rotator() + FRotator(
                FMath::RandRange(-LayerConfig.RotationDelta.Pitch, LayerConfig.RotationDelta.Pitch),
                FMath::RandRange(-LayerConfig.RotationDelta.Yaw, LayerConfig.RotationDelta.Yaw),
                FMath::RandRange(-LayerConfig.RotationDelta.Roll, LayerConfig.RotationDelta.Roll));
            InstanceInfo.Scale = ActualTransform.GetScale3D();

            ScatteredInstances.SmallObjects.Add(InstanceInfo);
            SuccessCount++;
        }
    }

    return SuccessCount;
}

bool ARandomScatterDA::GenerateAttachedLocation(const FScatteredInstanceInfo& TargetBuilding, FVector& OutLocation) const
{
    FVector BuildingLocation = TargetBuilding.Location;

    bool bAttachToTop = FMath::RandRange(0.0f, 1.0f) < 0.3f;
    float SearchRadius = 200.0f;

    if (bAttachToTop) {
        OutLocation = FVector(
            BuildingLocation.X + FMath::RandRange(-SearchRadius * 0.5f, SearchRadius * 0.5f),
            BuildingLocation.Y + FMath::RandRange(-SearchRadius * 0.5f, SearchRadius * 0.5f),
            BuildingLocation.Z + FMath::RandRange(100.0f, 200.0f));
    }
    else {
        float Side = FMath::RandRange(0.0f, 4.0f);
        float HeightRatio = FMath::RandRange(0.2f, 0.8f);

        if (Side < 1.0f) {
            OutLocation = FVector(BuildingLocation.X + SearchRadius, BuildingLocation.Y + FMath::RandRange(-SearchRadius * 0.5f, SearchRadius * 0.5f), BuildingLocation.Z + 100.0f * HeightRatio);
        }
        else if (Side < 2.0f) {
            OutLocation = FVector(BuildingLocation.X - SearchRadius, BuildingLocation.Y + FMath::RandRange(-SearchRadius * 0.5f, SearchRadius * 0.5f), BuildingLocation.Z + 100.0f * HeightRatio);
        }
        else if (Side < 3.0f) {
            OutLocation = FVector(BuildingLocation.X + FMath::RandRange(-SearchRadius * 0.5f, SearchRadius * 0.5f), BuildingLocation.Y + SearchRadius, BuildingLocation.Z + 100.0f * HeightRatio);
        }
        else {
            OutLocation = FVector(BuildingLocation.X + FMath::RandRange(-SearchRadius * 0.5f, SearchRadius * 0.5f), BuildingLocation.Y - SearchRadius, BuildingLocation.Z + 100.0f * HeightRatio);
        }
    }

    return true;
}

void ARandomScatterDA::GeneratePoissonPointsAroundBuilding(const FVector& BuildingLocation, float Radius, float MinDistance, int32 TargetCount, TArray<FVector>& OutPoints) const
{
    OutPoints.Empty();

    // 使用改进的泊松圆盘采样算法
    TArray<FVector> ActiveList;
    TSet<FIntPoint> Grid;

    float CellSize = MinDistance / FMath::Sqrt(2.0f);
    int32 GridWidth = FMath::CeilToInt(2 * Radius / CellSize);
    int32 GridHeight = GridWidth;

    // 添加中心点周围的初始点
    int32 InitialRingCount = 8;
    float InitialRadius = MinDistance * 1.5f;

    for (int32 i = 0; i < InitialRingCount; i++) {
        float Angle = (2.0f * PI * i) / InitialRingCount;
        FVector InitialPoint = BuildingLocation + FVector(
                                                      InitialRadius * FMath::Cos(Angle),
                                                      InitialRadius * FMath::Sin(Angle),
                                                      0.0f);

        // 检查是否在半径范围内
        if (FVector::Dist2D(InitialPoint, BuildingLocation) <= Radius) {
            OutPoints.Add(InitialPoint);
            ActiveList.Add(InitialPoint);

            // 添加到网格
            int32 GridX = FMath::FloorToInt((InitialPoint.X - (BuildingLocation.X - Radius)) / CellSize);
            int32 GridY = FMath::FloorToInt((InitialPoint.Y - (BuildingLocation.Y - Radius)) / CellSize);
            if (GridX >= 0 && GridX < GridWidth && GridY >= 0 && GridY < GridHeight) {
                Grid.Add(FIntPoint(GridX, GridY));
            }
        }
    }

    // 主采样循环
    int32 MaxAttempts = 30;
    while (ActiveList.Num() > 0 && OutPoints.Num() < TargetCount) {
        int32 RandomIndex = FMath::RandRange(0, ActiveList.Num() - 1);
        FVector CurrentPoint = ActiveList[RandomIndex];

        bool bFoundValidPoint = false;

        for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++) {
            // 在当前点周围生成候选点
            float Distance = FMath::RandRange(MinDistance, MinDistance * 2.0f);
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);

            FVector CandidatePoint = CurrentPoint + FVector(
                                                        Distance * FMath::Cos(Angle),
                                                        Distance * FMath::Sin(Angle),
                                                        0.0f);

            // 检查是否在搜索半径内
            if (FVector::Dist2D(CandidatePoint, BuildingLocation) > Radius) {
                continue;
            }

            // 检查网格位置
            int32 GridX = FMath::FloorToInt((CandidatePoint.X - (BuildingLocation.X - Radius)) / CellSize);
            int32 GridY = FMath::FloorToInt((CandidatePoint.Y - (BuildingLocation.Y - Radius)) / CellSize);

            if (GridX < 0 || GridX >= GridWidth || GridY < 0 || GridY >= GridHeight) {
                continue;
            }

            // 检查附近是否有其他点
            bool bTooClose = false;
            for (int32 CheckX = FMath::Max(0, GridX - 2); CheckX <= FMath::Min(GridWidth - 1, GridX + 2); CheckX++) {
                for (int32 CheckY = FMath::Max(0, GridY - 2); CheckY <= FMath::Min(GridHeight - 1, GridY + 2); CheckY++) {
                    if (Grid.Contains(FIntPoint(CheckX, CheckY))) {
                        // 找到该网格中的点并检查距离
                        for (const FVector& ExistingPoint : OutPoints) {
                            int32 ExistingGridX = FMath::FloorToInt((ExistingPoint.X - (BuildingLocation.X - Radius)) / CellSize);
                            int32 ExistingGridY = FMath::FloorToInt((ExistingPoint.Y - (BuildingLocation.Y - Radius)) / CellSize);

                            if (ExistingGridX == CheckX && ExistingGridY == CheckY) {
                                if (FVector::Dist2D(CandidatePoint, ExistingPoint) < MinDistance) {
                                    bTooClose = true;
                                    break;
                                }
                            }
                        }
                        if (bTooClose) break;
                    }
                }
                if (bTooClose) break;
            }

            if (!bTooClose) {
                OutPoints.Add(CandidatePoint);
                ActiveList.Add(CandidatePoint);
                Grid.Add(FIntPoint(GridX, GridY));
                bFoundValidPoint = true;
                break;
            }
        }

        if (!bFoundValidPoint) {
            ActiveList.RemoveAt(RandomIndex);
        }
    }

    // 如果点数不够，添加一些环形分布的点
    while (OutPoints.Num() < TargetCount) {
        float RingRadius = FMath::RandRange(MinDistance * 2.0f, Radius * 0.8f);
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);

        FVector RingPoint = BuildingLocation + FVector(
                                                   RingRadius * FMath::Cos(Angle),
                                                   RingRadius * FMath::Sin(Angle),
                                                   0.0f);

        // 检查与现有点的距离
        bool bValidRingPoint = true;
        for (const FVector& ExistingPoint : OutPoints) {
            if (FVector::Dist2D(RingPoint, ExistingPoint) < MinDistance * 0.8f) {
                bValidRingPoint = false;
                break;
            }
        }

        if (bValidRingPoint) {
            OutPoints.Add(RingPoint);
        }
        else {
            // 避免无限循环
            break;
        }
    }
}

FBox ARandomScatterDA::GetActualMeshBounds(UStaticMesh* Mesh, const FVector& Scale) const
{
    if (!Mesh) {
        return FBox(ForceInit);
    }

    // 获取mesh的原始bounds
    FBox OriginalBounds = Mesh->GetBounds().GetBox();

    // 应用scale计算实际bounds
    FVector Min = OriginalBounds.Min * Scale;
    FVector Max = OriginalBounds.Max * Scale;

    return FBox(Min, Max);
}

int32 ARandomScatterDA::CalculateSubBuildingCount(const FScatteredInstanceInfo& MainBuilding, const FLayerScatterConfig& LayerConfig) const
{
    // 基础数量
    int32 BaseCount = LayerConfig.Num;

    // 根据主建筑的实际尺寸计算multiplier
    double BuildingSize = MainBuilding.ActualBounds.GetVolume();

    // 定义基准面积
    float kBase = 1000000.0f;
    float SizeMultiplier = FMath::Sqrt(BuildingSize / kBase);

    // 限制multiplier范围在0.5到3.0之间
    SizeMultiplier = FMath::Clamp(SizeMultiplier, 0.f, 3.0f);

    int32 AdjustedCount = FMath::RoundToInt(BaseCount * SizeMultiplier);

    // 确保最少有1个，最多不超过基础数量的3倍
    return FMath::Clamp(AdjustedCount, 1, BaseCount * 3);
}

void ARandomScatterDA::GenerateBuildingClusters(const FVector& Min, const FVector& Max, const FLayerScatterConfig& LayerConfig, TArray<FVector>& OutLocations) const
{
    OutLocations.Empty();

    FVector AreaSize = Max - Min;

    // 直接生成具有聚集特征的建筑位置
    // 使用多个聚集核心点，每个核心点周围产生多个建筑

    // 确定聚集强度和聚集核心数量
    int32 ClusterCoreCount = FMath::Max(3, LayerConfig.Num / 4); // 聚集核心数量
    float ClusterInfluenceRadius = FMath::Min(AreaSize.X, AreaSize.Y) * 0.15f * ClusterInfluenceRadiusBase; // 每个聚集核心的影响半径

    // 生成聚集核心点（这些不是建筑位置，而是吸引建筑的"磁场"中心）
    TArray<FVector> ClusterCores;
    TArray<float> ClusterStrengths; // 每个核心的聚集强度

    for (int32 i = 0; i < ClusterCoreCount * 3; i++) {
        if (ClusterCores.Num() >= ClusterCoreCount) break;

        FVector CandidateCore = FVector(
            FMath::RandRange(Min.X, Max.X),
            FMath::RandRange(Min.Y, Max.Y),
            Min.Z);

        // 聚集核心之间保持一定距离
        bool bTooClose = false;
        for (const FVector& ExistingCore : ClusterCores) {
            if (FVector::Dist2D(CandidateCore, ExistingCore) < ClusterInfluenceRadius * 1.2f) {
                bTooClose = true;
                break;
            }
        }

        if (!bTooClose) {
            ClusterCores.Add(CandidateCore);
            // 随机聚集强度：有些核心吸引更多建筑，有些较少
            ClusterStrengths.Add(ClusterStrengthBase * FMath::RandRange(0.35f, 2.0f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("生成了 %d 个聚集核心点"), ClusterCores.Num());

    // 现在直接生成所有建筑位置，每个位置受到所有聚集核心的影响
    int32 MaxAttempts = LayerConfig.Num * 10;

    for (int32 Attempt = 0; Attempt < MaxAttempts && OutLocations.Num() < LayerConfig.Num; Attempt++) {
        FVector CandidateLocation;

        // 基于聚集核心的概率密度采样
        if (SampleLocationBasedOnClusterCores(Min, Max, ClusterCores, ClusterStrengths, ClusterInfluenceRadius, CandidateLocation)) {
            // 简单的重叠检查，允许一定程度的重叠
            bool bAcceptable = true;
            float MinAllowedDistance = 30.0f; // 很小的距离，主要防止完全重合

            for (const FVector& ExistingLocation : OutLocations) {
                if (FVector::Dist2D(CandidateLocation, ExistingLocation) < MinAllowedDistance) {
                    bAcceptable = false;
                    break;
                }
            }

            if (bAcceptable) {
                OutLocations.Add(CandidateLocation);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("建筑聚集采样完成：生成 %d 个建筑位置"), OutLocations.Num());
}

bool ARandomScatterDA::SampleLocationBasedOnClusterCores(const FVector& Min, const FVector& Max, const TArray<FVector>& ClusterCores, const TArray<float>& ClusterStrengths, float InfluenceRadius, FVector& OutLocation) const
{
    if (ClusterCores.Num() == 0) {
        // 如果没有聚集核心，就随机采样
        OutLocation = FVector(
            FMath::RandRange(Min.X, Max.X),
            FMath::RandRange(Min.Y, Max.Y),
            Min.Z);
        return true;
    }

    // 使用重要性采样：根据聚集核心的影响来采样位置
    // 首先选择一个聚集核心（基于强度的权重选择）

    float TotalWeight = 0.0f;
    for (float Strength : ClusterStrengths) {
        TotalWeight += Strength;
    }

    float RandomWeight = FMath::RandRange(0.0f, TotalWeight);
    int32 SelectedCoreIndex = 0;
    float AccumulatedWeight = 0.0f;

    for (int32 i = 0; i < ClusterCores.Num(); i++) {
        AccumulatedWeight += ClusterStrengths[i];
        if (RandomWeight <= AccumulatedWeight) {
            SelectedCoreIndex = i;
            break;
        }
    }

    const FVector& SelectedCore = ClusterCores[SelectedCoreIndex];
    float SelectedStrength = ClusterStrengths[SelectedCoreIndex];

    // 在选定的聚集核心周围采样
    // 使用变化的采样策略来增加多样性

    float SamplingType = FMath::RandRange(0.0f, 1.0f);

    if (SamplingType < 0.6f) {
        // 60%概率：高斯分布采样（紧密聚集）
        float GaussianDistance = FMath::Abs(GenerateGaussianRandom()) * InfluenceRadius * SelectedStrength * 0.4f;
        GaussianDistance = FMath::Min(GaussianDistance, InfluenceRadius * SelectedStrength);

        float Angle = FMath::RandRange(0.0f, 2.0f * PI);

        OutLocation = SelectedCore + FVector(
                                         GaussianDistance * FMath::Cos(Angle),
                                         GaussianDistance * FMath::Sin(Angle),
                                         0.0f);
    }
    else if (SamplingType < 0.85f) {
        // 25%概率：均匀圆形分布
        float Distance = FMath::RandRange(0.0f, InfluenceRadius * SelectedStrength * 0.7f);
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);

        OutLocation = SelectedCore + FVector(
                                         Distance * FMath::Cos(Angle),
                                         Distance * FMath::Sin(Angle),
                                         0.0f);
    }
    else {
        // 15%概率：多核心影响的混合采样
        // 同时受到多个核心的影响
        FVector WeightedPosition = FVector::ZeroVector;
        float TotalInfluence = 0.0f;

        for (int32 i = 0; i < ClusterCores.Num(); i++) {
            float Distance = FVector::Dist2D(SelectedCore, ClusterCores[i]);
            if (Distance < InfluenceRadius * 2.0f) {
                float Influence = ClusterStrengths[i] / FMath::Max(1.0f, Distance * 0.01f);
                WeightedPosition += ClusterCores[i] * Influence;
                TotalInfluence += Influence;
            }
        }

        if (TotalInfluence > 0.0f) {
            FVector CenterOfInfluence = WeightedPosition / TotalInfluence;

            // 在影响中心周围采样
            float Distance = FMath::RandRange(0.0f, InfluenceRadius * 0.5f);
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);

            OutLocation = CenterOfInfluence + FVector(
                                                  Distance * FMath::Cos(Angle),
                                                  Distance * FMath::Sin(Angle),
                                                  0.0f);
        }
        else {
            // 回退到简单的高斯采样
            float Distance = FMath::Abs(GenerateGaussianRandom()) * InfluenceRadius * 0.3f;
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);

            OutLocation = SelectedCore + FVector(
                                             Distance * FMath::Cos(Angle),
                                             Distance * FMath::Sin(Angle),
                                             0.0f);
        }
    }

    // 确保位置在有效范围内
    OutLocation.X = FMath::Clamp(OutLocation.X, Min.X, Max.X);
    OutLocation.Y = FMath::Clamp(OutLocation.Y, Min.Y, Max.Y);
    OutLocation.Z = Min.Z;

    return true;
}

float ARandomScatterDA::GenerateGaussianRandom() const
{
    // 使用Box-Muller变换生成标准正态分布随机数
    static bool bHasSpare = false;
    static float SpareValue = 0.0f;

    if (bHasSpare) {
        bHasSpare = false;
        return SpareValue;
    }

    bHasSpare = true;

    float U = FMath::RandRange(0.0f, 1.0f);
    float V = FMath::RandRange(0.0f, 1.0f);

    // 避免U为0导致log(0)的问题
    while (U <= 0.0f) {
        U = FMath::RandRange(0.0f, 1.0f);
    }

    float Magnitude = FMath::Sqrt(-2.0f * FMath::Loge(U));
    float Angle = 2.0f * PI * V;

    SpareValue = Magnitude * FMath::Sin(Angle);
    return Magnitude * FMath::Cos(Angle);
}