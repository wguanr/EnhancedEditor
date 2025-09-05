#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ScatterTypes.generated.h"

UENUM(BlueprintType)
enum class EScatterAlgorithm : uint8
{
    PoissonDisk2D UMETA(DisplayName = "Poisson Disk (2D)"),
    GridJitter2D UMETA(DisplayName = "Grid Jitter (2D)"),
    ClusterCores2D UMETA(DisplayName = "Clustered Cores (2D)"),
    SurfaceAreaWeighted UMETA(DisplayName = "Surface Area Weighted"),
    HexGrid2D UMETA(DisplayName = "Hex Grid (2D)")
};

USTRUCT(BlueprintType)
struct FScatterLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Num = 50;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDistance = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ScalarDelta = FVector(0.1f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationDelta = FRotator(10.f, 45.f, 10.f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D HeightOffsetRange = FVector2D(0.f, 0.f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAlignToGround = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroundTraceDistance = 10000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClusterCoreCount = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterInfluenceRadius = 2000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Jitter = 0.35f;
};

// used for scatter algo， represent the former HISM's attributes
USTRUCT(BlueprintType)
struct FScatterContext
{
    GENERATED_BODY()
    UPROPERTY()
    FTransform Transform;
    UPROPERTY()
    FBox Bound;
    UPROPERTY()
    float floatHandle = 0.f;
    UPROPERTY()
    int intHandle = -1;
    UPROPERTY()
    FName tagHandle = NAME_None;
};

// cached for next layer's scatter
USTRUCT(BlueprintType)
struct FScatterOutputs
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    uint8 LayerIndex = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FScatterContext> InstanceScatterContext;
};

// used for hism gen, pick elements
USTRUCT(BlueprintType)
struct FScatterAssetEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UMaterialInterface>> CandidateMaterials;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Tag = NAME_None;
};

USTRUCT(BlueprintType)
struct FScatterAssetPool
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FScatterAssetEntry> Entries;
};

USTRUCT(BlueprintType)
struct FPickedAsset
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMesh *Mesh = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UMaterialInterface *Material = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName Tag = NAME_None;
};
