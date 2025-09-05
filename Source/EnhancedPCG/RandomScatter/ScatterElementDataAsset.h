// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "ScatterElementDataAsset.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class ENHANCEDPCG_API UScatterElementDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain"))
    TArray<TSoftObjectPtr<UStaticMesh>> Meshes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Builds1"))
    TArray<TSoftObjectPtr<UStaticMesh>> Meshes1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Builds2"))
    TArray<TSoftObjectPtr<UStaticMesh>> Meshes2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain_Mat"))
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain_Mat1"))
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BuildsMain_Mat2"))
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials2;

    // PowerPole 资源池（可选）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "PowerPole_Meshes"))
    TArray<TSoftObjectPtr<UStaticMesh>> PoleMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "PowerPole_Materials"))
    TArray<TSoftObjectPtr<UMaterialInterface>> PoleMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "PowerPole_Socket_Meshes"))
    TArray<TSoftObjectPtr<UStaticMesh>> PoleSocketMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "PowerPole_Socket_Materials"))
    TArray<TSoftObjectPtr<UMaterialInterface>> PoleSocketMaterials;
};
