#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ScatterTypes.h"
#include "ScatterAlgorithm.h"
#include "ScatterActorBase.generated.h"

UCLASS(Blueprintable)
class ENHANCEDPCG_API AScatterActorBase : public AActor
{
    GENERATED_BODY()
public:
    AScatterActorBase();

    // 统一资产池（优先）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Assets")
    FScatterAssetPool AssetPool;

    // 算法与配置（以策略对象替代枚举）
    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Scatter|Config")
    UScatterAlgorithm *AlgorithmStrategy = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Config")
    FScatterLayerConfig Config;

    // 边界与附着输入
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Bounds")
    AVolume * ScatterBound;
    
    // Editor 入口
    UFUNCTION(CallInEditor, Category = "Scatter")
    void ScatterThis();

    // 跨层接口（基于 FScatterContext）
    virtual void AcceptPreviousContexts(const TArray<FScatterContext> &InContexts);

    UFUNCTION(BlueprintCallable, Category = "Scatter")
    virtual const FScatterOutputs &GetOutputs() const { return Outputs; }

    UFUNCTION(CallInEditor, Category = "Scatter")
    virtual void ClearPreviousScatter();

protected:
    virtual void BeginPlay() override;

protected:
    // 统一散布入口（基于上下文列表）
    virtual void DoScatter(const TArray<FScatterContext> &InPrevContexts, FScatterOutputs &Out);

protected:
    // 资产准备与选择
    virtual bool PrepareAssets();
    virtual bool PickAsset(FPickedAsset &Out, const FName TagFilter = NAME_None);

    // HISM 工具
    virtual UHierarchicalInstancedStaticMeshComponent *GetOrCreateHISMC(UStaticMesh *Mesh, UMaterialInterface *Material);
    virtual UHierarchicalInstancedStaticMeshComponent *GetOrCreateHISMCByPicked(const FPickedAsset &Picked);
    virtual int32 AddInstanceWithRandomizedTransform(UHierarchicalInstancedStaticMeshComponent *HISMC, const FVector &Location, const FRotator *InBaseRotation = nullptr, const FVector *InBaseScale = nullptr);

protected:
    // 缓存
    TArray<FScatterContext> PreviousContexts;
    FScatterOutputs Outputs;

    // 运行时缓存已加载资产
    TArray<UStaticMesh *> LoadedMeshes;
    TArray<UMaterialInterface *> LoadedMaterials;

    // HISM 分组：键为 Mesh+Material 指针字符串
    TMap<FString, UHierarchicalInstancedStaticMeshComponent *> MeshMatToHISMC;

private:
    FString MakeKey(UStaticMesh *Mesh, UMaterialInterface *Material) const;
};
