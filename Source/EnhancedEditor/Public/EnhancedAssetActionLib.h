// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blutility/Classes/AssetActionUtility.h"
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
