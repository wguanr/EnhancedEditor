#include "ScatterActorBase.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/EngineTypes.h"

AScatterActorBase::AScatterActorBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AScatterActorBase::BeginPlay()
{
    Super::BeginPlay();
}

void AScatterActorBase::ScatterThis()
{
    // 清理上次结果
    ClearPreviousScatter();

    // 准备资产
    if (!PrepareAssets())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s PrepareAssets failed"), *GetName());
    }

    // 执行散布（基于上一层的 FScatterContext 列表）
    DoScatter(PreviousContexts, Outputs);
}
void AScatterActorBase::DoScatter(const TArray<FScatterContext> &InPrevContexts, FScatterOutputs &Out)
{
    // 1) 聚合 Bounds2D：优先使用场景中的 ScatterBounds 体积集合
    FBox AggregatedBounds3D(ForceInit);

            if (IsValid(ScatterBound))
            {
                AggregatedBounds3D = ScatterBound->GetComponentsBoundingBox(true);
            }
    

    const FVector Min3 = AggregatedBounds3D.Min;
    const FVector Max3 = AggregatedBounds3D.Max;
    const FBox2D Bounds2D(FVector2D(Min3.X, Min3.Y), FVector2D(Max3.X, Max3.Y));
    if (!Bounds2D.bIsValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s DoScatter: invalid 2D bounds"), *GetName());
        return;
    }

    // 2) 采样二维候选点：优先使用策略对象；无策略则回退到 PoissonDisk2D
    TArray<FVector2D> Points2D;
    if (AlgorithmStrategy)
    {
        AlgorithmStrategy->Generate(Bounds2D, InPrevContexts, Config, Points2D, Config.Seed);
    }
    else
    {
        // 无策略时使用默认 Poisson 策略
        if (UScatterAlgorithm_PoissonDisk *DefaultAlgo = NewObject<UScatterAlgorithm_PoissonDisk>(this))
        {
            DefaultAlgo->Generate(Bounds2D, InPrevContexts, Config, Points2D, Config.Seed);
        }
    }

    // 3) 将 2D 点映射为 3D 世界位置（Z 可对齐地面），实例化 HISM，并写出 FScatterContext
    UWorld *World = GetWorld();
    if (!World)
    {
        return;
    }

    Out.InstanceScatterContext.Reset();
    Out.InstanceScatterContext.Reserve(Points2D.Num());

    for (const FVector2D &P2 : Points2D)
    {
        FVector WorldPos(P2.X, P2.Y, AggregatedBounds3D.Max.Z);

        if (Config.bAlignToGround)
        {
            FHitResult Hit;
            const FVector Start(WorldPos + FVector(0, 0, Config.GroundTraceDistance * 0.5f));
            const FVector End(WorldPos - FVector(0, 0, Config.GroundTraceDistance));
            FCollisionQueryParams Params(SCENE_QUERY_STAT(ScatterAlign), false, this);
            if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
            {
                WorldPos = Hit.ImpactPoint;
            }
        }

        FPickedAsset Picked;
        if (!PickAsset(Picked))
        {
            continue;
        }
        UHierarchicalInstancedStaticMeshComponent *HISMC = GetOrCreateHISMCByPicked(Picked);
        if (!HISMC)
        {
            continue;
        }

        const int32 Index = AddInstanceWithRandomizedTransform(HISMC, WorldPos);
        if (Index >= 0)
        {
            FTransform InstanceXform;
            HISMC->GetInstanceTransform(Index, InstanceXform);

            FScatterContext Ctx;
            Ctx.Transform = InstanceXform;
            Ctx.tagHandle = Picked.Tag;
            // Bound 可按需补充为实例外包围盒，暂留默认
            Out.InstanceScatterContext.Add(MoveTemp(Ctx));
        }
    }
}
void AScatterActorBase::AcceptPreviousContexts(const TArray<FScatterContext> &InContexts)
{
    PreviousContexts = InContexts;
}

void AScatterActorBase::ClearPreviousScatter()
{
    Outputs = FScatterOutputs();

    for (auto &Pair : MeshMatToHISMC)
    {
        if (UHierarchicalInstancedStaticMeshComponent *HISMC = Pair.Value)
        {
            HISMC->ClearInstances();
            HISMC->DestroyComponent();
        }
    }
    MeshMatToHISMC.Empty();
}

bool AScatterActorBase::PrepareAssets()
{
    LoadedMeshes.Empty();
    LoadedMaterials.Empty();

    // 仅使用统一资产池
    for (const FScatterAssetEntry &Entry : AssetPool.Entries)
    {
        if (UStaticMesh *Mesh = Entry.Mesh.LoadSynchronous())
        {
            LoadedMeshes.AddUnique(Mesh);
            for (const TSoftObjectPtr<UMaterialInterface> &MatSoft : Entry.CandidateMaterials)
            {
                if (UMaterialInterface *Mat = MatSoft.LoadSynchronous())
                {
                    LoadedMaterials.AddUnique(Mat);
                }
            }
        }
    }

    return LoadedMeshes.Num() > 0;
}

bool AScatterActorBase::PickAsset(FPickedAsset &Out, const FName TagFilter)
{
    // 优先从统一资产池按权重抽样
    if (AssetPool.Entries.Num() > 0)
    {
        TArray<int32> CandidateIndices;
        TArray<float> Weights;
        for (int32 Index = 0; Index < AssetPool.Entries.Num(); ++Index)
        {
            const FScatterAssetEntry &Entry = AssetPool.Entries[Index];
            if (TagFilter.IsNone() || Entry.Tag == TagFilter)
            {
                if (Entry.Mesh.ToSoftObjectPath().IsValid())
                {
                    CandidateIndices.Add(Index);
                    Weights.Add(FMath::Max(Entry.Weight, 0.001f));
                }
            }
        }
        if (CandidateIndices.Num() > 0)
        {
            // 基于权重的轮盘赌选择
            int32 PickIdx = CandidateIndices[0];
            if (Weights.Num() == CandidateIndices.Num())
            {
                float TotalWeight = 0.f;
                for (float W : Weights)
                {
                    TotalWeight += FMath::Max(W, 0.0f);
                }
                if (TotalWeight > 0.f)
                {
                    const float Threshold = FMath::FRandRange(0.f, TotalWeight);
                    float Accum = 0.f;
                    for (int32 i = 0; i < Weights.Num(); ++i)
                    {
                        Accum += FMath::Max(Weights[i], 0.0f);
                        if (Accum >= Threshold)
                        {
                            PickIdx = CandidateIndices[i];
                            break;
                        }
                    }
                }
                else
                {
                    PickIdx = CandidateIndices[FMath::RandRange(0, CandidateIndices.Num() - 1)];
                }
            }
            const FScatterAssetEntry &PickedEntry = AssetPool.Entries[PickIdx];
            Out.Mesh = PickedEntry.Mesh.LoadSynchronous();
            Out.Tag = PickedEntry.Tag;
            if (PickedEntry.CandidateMaterials.Num() > 0)
            {
                const int32 MatIdx = FMath::RandRange(0, PickedEntry.CandidateMaterials.Num() - 1);
                Out.Material = PickedEntry.CandidateMaterials[MatIdx].LoadSynchronous();
            }
            else if (LoadedMaterials.Num() > 0)
            {
                Out.Material = LoadedMaterials[FMath::RandRange(0, LoadedMaterials.Num() - 1)];
            }
            else if (Out.Mesh)
            {
                Out.Material = Out.Mesh->GetMaterial(0);
            }
            return Out.Mesh != nullptr;
        }
    }

    // 资产池为空则失败
    return false;
}

UHierarchicalInstancedStaticMeshComponent *AScatterActorBase::GetOrCreateHISMC(UStaticMesh *Mesh, UMaterialInterface *Material)
{
    if (!Mesh)
    {
        return nullptr;
    }
    const FString Key = MakeKey(Mesh, Material);
    if (UHierarchicalInstancedStaticMeshComponent **Found = MeshMatToHISMC.Find(Key))
    {
        return *Found;
    }

    UHierarchicalInstancedStaticMeshComponent *HISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
    HISMC->SetupAttachment(RootComponent);
    HISMC->RegisterComponent();
    HISMC->SetStaticMesh(Mesh);
    if (Material)
    {
        HISMC->SetMaterial(0, Material);
    }
    MeshMatToHISMC.Add(Key, HISMC);
    return HISMC;
}

UHierarchicalInstancedStaticMeshComponent *AScatterActorBase::GetOrCreateHISMCByPicked(const FPickedAsset &Picked)
{
    return GetOrCreateHISMC(Picked.Mesh, Picked.Material);
}

int32 AScatterActorBase::AddInstanceWithRandomizedTransform(UHierarchicalInstancedStaticMeshComponent *HISMC, const FVector &Location, const FRotator *InBaseRotation, const FVector *InBaseScale)
{
    if (!HISMC)
    {
        return INDEX_NONE;
    }
    FVector Scale = InBaseScale ? *InBaseScale : FVector::OneVector;
    const FVector Jitter(
        FMath::Clamp(FMath::FRandRange(-Config.ScalarDelta.X, Config.ScalarDelta.X), -0.95f, 0.95f),
        FMath::Clamp(FMath::FRandRange(-Config.ScalarDelta.Y, Config.ScalarDelta.Y), -0.95f, 0.95f),
        FMath::Clamp(FMath::FRandRange(-Config.ScalarDelta.Z, Config.ScalarDelta.Z), -0.95f, 0.95f));
    Scale *= (FVector::OneVector + Jitter);
    Scale.X = FMath::Max(Scale.X, KINDA_SMALL_NUMBER);
    Scale.Y = FMath::Max(Scale.Y, KINDA_SMALL_NUMBER);
    Scale.Z = FMath::Max(Scale.Z, KINDA_SMALL_NUMBER);

    FRotator Rot = InBaseRotation ? *InBaseRotation : FRotator::ZeroRotator;
    Rot.Pitch += FMath::FRandRange(-Config.RotationDelta.Pitch, Config.RotationDelta.Pitch);
    Rot.Yaw += FMath::FRandRange(-Config.RotationDelta.Yaw, Config.RotationDelta.Yaw);
    Rot.Roll += FMath::FRandRange(-Config.RotationDelta.Roll, Config.RotationDelta.Roll);
    Rot.Normalize();

    float ZOffset = FMath::FRandRange(Config.HeightOffsetRange.X, Config.HeightOffsetRange.Y);
    FTransform Xform(Rot, Location + FVector(0, 0, ZOffset), Scale);
    return HISMC->AddInstance(Xform);
}

FString AScatterActorBase::MakeKey(UStaticMesh *Mesh, UMaterialInterface *Material) const
{
    return FString::Printf(TEXT("%p_%p"), Mesh, Material);
}
