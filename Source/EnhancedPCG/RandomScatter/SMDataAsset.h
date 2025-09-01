// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include  "Engine/StaticMesh.h"
#include "SMDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BLOCKS_API USMDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain"))
    TArray<TSoftObjectPtr<UStaticMesh>> Meshes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (DisplayName = "Builds1"))
    TArray<TSoftObjectPtr<UStaticMesh>> Meshes1;
        UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Builds2"))
        TArray<TSoftObjectPtr<UStaticMesh>> Meshes2;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain_Mat"))
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain_Mat1"))
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain_Mat2"))
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials2;
    
    
};
