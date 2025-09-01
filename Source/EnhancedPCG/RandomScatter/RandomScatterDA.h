#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FScatterData.h"
#include "RandomScatterDA.generated.h"

class USMDataAsset;
class AVolume;
class UMaterialInstance;

constexpr int kBuildsLevel = 3;

// 简化的散布对象信息结构
USTRUCT()
struct FScatteredInstanceInfo
{
    GENERATED_BODY()

    UPROPERTY()
    int32 InstanceIndex;

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    EScatterLevel Level;

    UPROPERTY()
    UHierarchicalInstancedStaticMeshComponent* HISMComponent;

    // 添加实际尺寸和旋转信息
    UPROPERTY()
    FBox ActualBounds;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    FVector Scale;

    FScatteredInstanceInfo()
    {
        InstanceIndex = -1;
        Location = FVector::ZeroVector;
        Level = EScatterLevel::MainBuilding;
        HISMComponent = nullptr;
        ActualBounds = FBox(ForceInit);
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
    }

    // 为TSet支持添加比较操作符
    bool operator==(const FScatteredInstanceInfo& Other) const
    {
        return InstanceIndex == Other.InstanceIndex &&
               HISMComponent == Other.HISMComponent &&
               Level == Other.Level &&
               Location.Equals(Other.Location, 1.0f); // 1.0f tolerance for float comparison
    }

    bool operator!=(const FScatteredInstanceInfo& Other) const
    {
        return !(*this == Other);
    }

    // 为TSet支持添加哈希函数
    friend uint32 GetTypeHash(const FScatteredInstanceInfo& Instance)
    {
        uint32 Hash = 0;
        Hash = HashCombine(Hash, ::GetTypeHash(Instance.InstanceIndex));
        Hash = HashCombine(Hash, ::GetTypeHash(Instance.HISMComponent));
        Hash = HashCombine(Hash, ::GetTypeHash(static_cast<int32>(Instance.Level)));

        // 手动计算FVector的哈希值
        Hash = HashCombine(Hash, ::GetTypeHash(Instance.Location.X));
        Hash = HashCombine(Hash, ::GetTypeHash(Instance.Location.Y));
        Hash = HashCombine(Hash, ::GetTypeHash(Instance.Location.Z));

        return Hash;
    }
};

// HISMC组件管理结构
USTRUCT()
struct FHISMCInfo
{
    GENERATED_BODY()

    UPROPERTY()
    UHierarchicalInstancedStaticMeshComponent* Component;

    UPROPERTY()
    UStaticMesh* Mesh;

    UPROPERTY()
    UMaterialInterface* Material;

    FHISMCInfo()
    {
        Component = nullptr;
        Mesh = nullptr;
        Material = nullptr;
    }
};

USTRUCT()
struct FScatteredInstances
{
    GENERATED_BODY()

    UPROPERTY()
    TSet<FScatteredInstanceInfo> MainBuildings;

    UPROPERTY()
    TSet<FScatteredInstanceInfo> SubBuildings;

    UPROPERTY()
    TSet<FScatteredInstanceInfo> SmallObjects;

    TSet<FScatteredInstanceInfo> Get(EScatterLevel Level) const
    {
        switch (Level) {
        case EScatterLevel::MainBuilding:
            return MainBuildings;
        case EScatterLevel::SubBuilding:
            return SubBuildings;
        case EScatterLevel::SmallObject:
            return SmallObjects;
        default:
            return TSet<FScatteredInstanceInfo>();
        }
    }

    int32 Num() const
    {
        return MainBuildings.Num() + SubBuildings.Num() + SmallObjects.Num();
    }

    void Empty()
    {
        MainBuildings.Empty();
        SubBuildings.Empty();
        SmallObjects.Empty();
    }
    
};

UCLASS(Blueprintable)
class BLOCKS_API ARandomScatterDA : public AActor
{
    GENERATED_BODY()

public:
    ARandomScatterDA();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AVolume*> ScatterBounds;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter Settings")
    TSoftObjectPtr<USMDataAsset> ScattersDataAsset;

    // 三层散布配置 - 支持蓝图编辑和JSON读取
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Settings")
    FLayerScatterConfig MainBuildingLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Settings")
    FLayerScatterConfig SubBuildingLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Settings")
    FLayerScatterConfig SmallObjectLayer;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Settings")
    float ClusterInfluenceRadiusBase = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Settings")
    float ClusterStrengthBase = 1;
    
    UFUNCTION(CallInEditor)
    void ScatterActors();

private:
    // HISMC组件管理
    UPROPERTY()
    TArray<FHISMCInfo> HISMComponents;

    // 存储已散布的实例信息, 0: main,1:sub
    FScatteredInstances ScatteredInstances;

    // ===== 三层散布系统函数 =====

    // 三层散布主函数
    void ScatterActorsLayered(const AVolume* Bounds);

    // 第一层：主体建筑散布（落在地面）
    void ScatterMainBuildings(const FVector& Min, const FVector& Max);

    // 第二层：附属构筑物散布（落在主建筑上）
    void ScatterSubBuildings();

    // 第三层：小物体散布（随机散布，避免碰撞）
    void ScatterSmallObjects(const FVector& Min, const FVector& Max);

    // ===== 辅助函数 =====

    // 从DataAsset获取网格和材质
    void GetMeshFromDataAsset(TArray<UStaticMesh*>& OutAssets, EScatterLevel Level) const;
    void GetMaterialsFromDataAsset(TArray<UMaterialInterface*>& OutMaterials, EScatterLevel Level) const;

    // 读取JSON配置
    void ReadJson();

    // 地面对齐 - 射线检测找到地面高度
    bool AlignToGround(FVector& InOutLocation, float TraceDistance = 10000.0f) const;

    // 检查碰撞
    bool CheckCollision(const FVector& Location, float CollisionRadius, EScatterLevel CurrentLevel) const;

    // 生成随机位置
    void GenerateRandomLocations(const FVector& Min, const FVector& Max, const FLayerScatterConfig& LayerConfig, TArray<FVector>& OutLocations) const;

    // 生成建筑组团位置（多阶段采样）
    void GenerateBuildingClusters(const FVector& Min, const FVector& Max, const FLayerScatterConfig& LayerConfig, TArray<FVector>& OutLocations) const;

    // 基于聚集核心的概率密度采样
    bool SampleLocationBasedOnClusterCores(const FVector& Min, const FVector& Max, const TArray<FVector>& ClusterCores, const TArray<float>& ClusterStrengths, float InfluenceRadius, FVector& OutLocation) const;

    // 生成高斯分布随机数
    float GenerateGaussianRandom() const;

    // HISMC管理
    UHierarchicalInstancedStaticMeshComponent* GetOrCreateHISMC(UStaticMesh* Mesh, UMaterialInterface* Material);
    int32 AddInstance(UHierarchicalInstancedStaticMeshComponent* HISMC, const FVector& Location, const FLayerScatterConfig& LayerConfig);
    int32 AddInstanceWithTransform(UHierarchicalInstancedStaticMeshComponent* HISMC, const FVector& Location, const FLayerScatterConfig& LayerConfig, FTransform& OutTransform);

    // 清理
    void ClearPreviousScatter();

    // ===== 小物体散布辅助函数 =====

    // 附着小物体散布（70%）
    int32 ScatterAttachedSmallObjects(int32 TargetCount, const TArray<UStaticMesh*>& StaticMeshes, const TArray<UMaterialInterface*>& Materials, const FLayerScatterConfig& LayerConfig);

    // 随机小物体散布（30%）
    int32 ScatterRandomSmallObjects(const FVector& Min, const FVector& Max, int32 TargetCount, const TArray<UStaticMesh*>& StaticMeshes, const TArray<UMaterialInterface*>& Materials, const FLayerScatterConfig& LayerConfig);

    // 生成附着在建筑上的位置
    bool GenerateAttachedLocation(const FScatteredInstanceInfo& TargetBuilding, FVector& OutLocation) const;

    // 泊松圆盘采样 - 在主建筑周围生成均匀分布的点位
    void GeneratePoissonPointsAroundBuilding(const FVector& BuildingLocation, float Radius, float MinDistance, int32 TargetCount, TArray<FVector>& OutPoints) const;

    // 计算实际的mesh bounds（考虑scale）
    FBox GetActualMeshBounds(UStaticMesh* Mesh, const FVector& Scale) const;

    // 根据MainBuilding尺寸计算SubBuilding数量
    int32 CalculateSubBuildingCount(const FScatteredInstanceInfo& MainBuilding, const FLayerScatterConfig& LayerConfig) const;
};