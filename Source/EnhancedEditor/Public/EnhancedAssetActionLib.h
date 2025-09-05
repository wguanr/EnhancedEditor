// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1
#include "Blutility/Classes/AssetActionUtility.h"
#include "AssetRegistryHelpers.h"
#else
#include "AssetActionUtility.h"
#endif
#include "EnhancedAssetActionLib.generated.h"



/**
 * 
 */
UCLASS()
class ENHANCEDEDITOR_API UEnhancedAssetActionLib : public UAssetActionUtility
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable,CallInEditor, Category = "Enhanced Asset Action")
    void FixMeshCollision(UStaticMesh* StaticMesh);

    UFUNCTION(BlueprintCallable,CallInEditor, Category = "Enhanced Asset Action")
    TArray<FAssetData> FilterThinMesh(TArray<FAssetData> InAssets, float Thin_limit, float Vol_limit, const FString& suffix);
    
};
