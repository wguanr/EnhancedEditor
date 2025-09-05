#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScatterTypes.h"
#include "ScatterActorBase.h"
#include "ScatterManager.generated.h"

UCLASS(Blueprintable)
class ENHANCEDPCG_API AScatterManager : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite, Category = "Scatter|Pipeline")
    TArray<AScatterActorBase *> ScatterLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Pipeline")
    bool bClearBeforeRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Pipeline")
    int32 GlobalSeed = 0;

    UFUNCTION(CallInEditor, Category = "Scatter")
    void RunPipeline();

    UFUNCTION(CallInEditor, Category = "Scatter")
    void ClearAll();

    // 为兼容旧的简化输入，提供集中注入入口（可选）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Assets")
    FScatterAssetPool DefaultAssetPool;

    // 简化输入（将被转换为 DefaultAssetPool）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> DefaultMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> DefaultMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Assets")
    FName DefaultTagForInjectedEntries = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Assets")
    float DefaultWeightForInjectedEntries = 1.f;
};
