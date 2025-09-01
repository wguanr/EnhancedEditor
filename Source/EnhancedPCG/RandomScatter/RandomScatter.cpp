// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomScatter.h"
#include "AssetRegistryModule.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"

#include "GameFramework/Volume.h"
#include "Kismet/GameplayStatics.h"

void ARandomScatter::BeginPlay()
{
    Super::BeginPlay();

    const FString MatFolderPath = TEXT("/Game/Megascans/Surfaces");
    const FString MeshFolderPath = TEXT("/Game/GeneratedWK/_GENERATED/Autel");
    ReadJson();
    // get post process volume
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVolume::StaticClass(), FoundActors);

    if (FoundActors.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("No volume found"));
        return;
    }

    for (AActor* Actor : FoundActors) {
        if (APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(Actor)) {
            ScatterActors(PostProcessVolume, MatFolderPath, MeshFolderPath);
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT("No post process volume found"));
        }
    }
}

ARandomScatter::ARandomScatter()
{
    ReadJson();
}

void ARandomScatter::ScatterActors(const AVolume* Bounds, const FString& MatFolderPath, const FString& MeshFolderPath)
{
    // 获取bounds的范围参数xyz
    FVector Origin = Bounds->GetBounds().Origin;
    FVector BoxExtent = Bounds->GetBounds().BoxExtent;

    if (BoxExtent == FVector::ZeroVector) {
        UE_LOG(LogTemp, Error, TEXT("Bounds is not valid"));
        return;
    }
    FVector Min = Origin - BoxExtent;
    FVector Max = Origin + BoxExtent;
    UE_LOG(LogTemp, Warning, TEXT("Min: %s, Max: %s"), *Min.ToString(), *Max.ToString());

    TArray<UMaterialInstance*> Material;

    TArray<FVector> RandomLocations;
    // Define the grid cell size
    const float CellSize = FMath::Min(BoxExtent.X, BoxExtent.Y) / (ScatterData.Num * 0.3);
    UE_LOG(LogTemp, Warning, TEXT("CellSize: %f"), CellSize);
    // Create a map to store locations by grid cell
    TMap<FIntVector, TArray<FVector>> Grid;

    for (int i = 0; i < ScatterData.Num; i++) {
        FVector _Loc;
        bool bLocationValid;
        do {
            bLocationValid = true;
            _Loc = FVector(FMath::RandRange(Min.X, Max.X),
                           FMath::RandRange(Min.Y, Max.Y),
                           FMath::RandRange(Min.Z, Min.Z + 1000));

            // Calculate the grid cell for the new location
            FIntVector Cell = FIntVector(FMath::FloorToInt(_Loc.X / CellSize),
                                         FMath::FloorToInt(_Loc.Y / CellSize),
                                         FMath::FloorToInt(_Loc.Z / CellSize));
            // Check nearby cells for existing locations
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        FIntVector NeighborCell = Cell + FIntVector(dx, dy, dz);
                        if (Grid.Contains(NeighborCell)) {
                            for (const FVector& ExistingLoc : Grid[NeighborCell]) {
                                if (FVector::Dist(_Loc, ExistingLoc) < CellSize * 2.5) {
                                    bLocationValid = false;
                                    break;
                                }
                            }
                        }
                        if (!bLocationValid) break;
                    }
                    if (!bLocationValid) break;
                }
                if (!bLocationValid) break;
            }
        } while (!bLocationValid);

        // Add the location to the grid
        FIntVector Cell = FIntVector(FMath::FloorToInt(_Loc.X / CellSize),
                                     FMath::FloorToInt(_Loc.Y / CellSize),
                                     FMath::FloorToInt(_Loc.Z / CellSize));
        Grid.FindOrAdd(Cell).Add(_Loc);
        RandomLocations.Add(_Loc);
    }

    if (RandomLocations.Num() != ScatterData.Num) {
        UE_LOG(LogTemp, Error, TEXT("RandomLocations Num: %d, ScatterData Num: %d"), RandomLocations.Num(), ScatterData.Num);
        return;
    }

    GetMaterialsFromFolder(MatFolderPath, Material);

    TArray<UStaticMesh*> StaticMeshes;
    GetMeshFromFolder(MeshFolderPath, StaticMeshes);

    // 生成随机位置, 均匀散布
    GenRandomMeshComp(ScatterData.Num, RandomLocations, Material, StaticMeshes);

}

TObjectPtr<AActor> ARandomScatter::GenRandomActor(const TSubclassOf<AActor>& ActorClass, const FVector& Location, TArray<UMaterialInterface*> Material)
{
    FRotator Rotation = FRotator(FMath::RandRange(-ScatterData.RotationDelta.Pitch, ScatterData.RotationDelta.Pitch),
                                 FMath::RandRange(-ScatterData.RotationDelta.Yaw, ScatterData.RotationDelta.Yaw),
                                 FMath::RandRange(-ScatterData.RotationDelta.Roll, ScatterData.RotationDelta.Roll));
    TObjectPtr<AActor> Actor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::Zero(), Rotation);// the final actor to spawn

    FVector Scale = FVector(FMath::RandRange(1 - ScatterData.ScalarDelta.X, 1 + ScatterData.ScalarDelta.X),
                            FMath::RandRange(1 - ScatterData.ScalarDelta.Y, 1 + ScatterData.ScalarDelta.Y),
                            FMath::RandRange(1 - ScatterData.ScalarDelta.Z, 1 + ScatterData.ScalarDelta.Z));
    Actor->SetActorScale3D(Actor->GetActorScale3D() + Scale);

    // set material
    TSet<UActorComponent*> MeshComponent = Actor->GetComponents();
    if (MeshComponent.Num() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Actor has no mesh component"));
        return Actor;
    }
    for (UActorComponent* Component : MeshComponent) {
        if (UStaticMeshComponent* MC = Cast<UStaticMeshComponent>(Component)) {
            if (Material.Num() > 0) {
                UMaterialInterface* PickMaterial = Material[FMath::RandRange(0, Material.Num() - 1)];
                MC->SetMaterial(0, PickMaterial);
            }
        }

    }

    return Actor;
}


void ARandomScatter::GetMaterialsFromFolder(const FString& MatFolderPath, TArray<UMaterialInstance*>& OutMaterials)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.ClassNames.Add(UMaterialInstance::StaticClass()->GetFName());
    Filter.PackagePaths.Add(*MatFolderPath);
    Filter.bRecursivePaths = true;
    Filter.bRecursiveClasses = true;

    TArray<FAssetData> AssetData;
    AssetRegistry.GetAssets(Filter, AssetData);

    for (const FAssetData& Data : AssetData) {
        if (UMaterialInstance* Asset = Cast<UMaterialInstance>(Data.GetAsset())) {
            OutMaterials.Add(Asset);
        }

    }
}


void ARandomScatter::GetMeshFromFolder(const FString& MeshFolderPath, TArray<UStaticMesh*>& OutAssets)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.ClassNames.Add(UStaticMesh::StaticClass()->GetFName());
    Filter.PackagePaths.Add(*MeshFolderPath);
    Filter.bRecursivePaths = true;

    TArray<FAssetData> AssetData;
    AssetRegistry.GetAssets(Filter, AssetData);

    for (const FAssetData& Data : AssetData) {
        UStaticMesh* Asset = Cast<UStaticMesh>(Data.GetAsset());
        if (Asset) {
            OutAssets.Add(Asset);
        }
    }

}

void ARandomScatter::GenRandomMeshComp(int CompNum, TArray<FVector> Location, TArray<UMaterialInstance*> Material, TArray<UStaticMesh*> StaticMeshes)
{
    // print all inputs
    UE_LOG(LogTemp, Warning, TEXT("CompNum: %d, Location: %d, Material: %d, StaticMeshes: %d"), CompNum, Location.Num(), Material.Num(), StaticMeshes.Num());

    checkf(CompNum != 0 && Location.Num() != 0 && Material.Num() != 0 && StaticMeshes.Num() != 0, TEXT("CompNum: %d, Location: %d, Material: %d, StaticMeshes: %d"), CompNum, Location.Num(), Material.Num(), StaticMeshes.Num());

    for (int i = 0; i < CompNum; i++) {
        FRotator Rotation = FRotator(FMath::RandRange(-ScatterData.RotationDelta.Pitch, ScatterData.RotationDelta.Pitch),
                                     FMath::RandRange(-ScatterData.RotationDelta.Yaw, ScatterData.RotationDelta.Yaw),
                                     FMath::RandRange(-ScatterData.RotationDelta.Roll, ScatterData.RotationDelta.Roll));
        float ScalarMultiplier = FMath::RandRange(1., 1.6);
        FVector Scale = FVector(ScalarMultiplier * FMath::RandRange(1 - ScatterData.ScalarDelta.X, 1 + ScatterData.ScalarDelta.X),
                                ScalarMultiplier * FMath::RandRange(1 - ScatterData.ScalarDelta.Y, 1 + ScatterData.ScalarDelta.Y),
                                ScalarMultiplier * FMath::RandRange(1 - ScatterData.ScalarDelta.Z, 1 + ScatterData.ScalarDelta.Z));

        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
        UStaticMesh* StaticMesh = StaticMeshes[FMath::RandRange(0, StaticMeshes.Num() - 1)];
        MeshComp->SetStaticMesh(StaticMesh);
        MeshComp->SetWorldLocation(Location[FMath::RandRange(0, Location.Num() - 1)]);

        MeshComp->SetWorldRotation(Rotation);
        MeshComp->SetWorldScale3D(Scale);
        MeshComp->SetMaterial(0, Material[FMath::RandRange(0, Material.Num() - 1)]);
        MeshComp->SetVisibility(true);
        MeshComp->bRenderCustomDepth = true;
        MeshComp->SetCustomDepthStencilValue(1);

        MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        MeshComp->RegisterComponent();

    }

}

void ARandomScatter::ReadJson()
{

    FString JsonString;
    FString JsonFilePath = FPaths::GameSourceDir() + TEXT("Blocks/Resources/Scatter.json");

    if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file"));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to load JSON file"));
        return;
    }

    // read json file one by one
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
        ScatterData.Num = JsonObject->GetIntegerField("Num");
        JsonObject->GetObjectField("ScalarDelta")->TryGetNumberField("X", ScatterData.ScalarDelta.X);
        JsonObject->GetObjectField("ScalarDelta")->TryGetNumberField("Y", ScatterData.ScalarDelta.Y);
        JsonObject->GetObjectField("ScalarDelta")->TryGetNumberField("Z", ScatterData.ScalarDelta.Z);
        JsonObject->GetObjectField("RotationDelta")->TryGetNumberField("Pitch", ScatterData.RotationDelta.Pitch);
        JsonObject->GetObjectField("RotationDelta")->TryGetNumberField("Yaw", ScatterData.RotationDelta.Yaw);
        JsonObject->GetObjectField("RotationDelta")->TryGetNumberField("Roll", ScatterData.RotationDelta.Roll);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON file"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Num: %d, ScalarDelta: %s, RotationDelta: %s"), ScatterData.Num, *ScatterData.ScalarDelta.ToString(), *ScatterData.RotationDelta.ToString());

}