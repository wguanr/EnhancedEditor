// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include  "Engine/StaticMesh.h"
#include "VoxScatterDataAsset.generated.h"


USTRUCT(BlueprintType)
struct FVoxScatterData
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    int64 seed = 0;
    // UPROPERTY(BlueprintReadWrite)
    // float initSize = 100.0f;
    UPROPERTY(BlueprintReadWrite)
    int64 iterations = 10;
};

/**
 * 
 */
UCLASS(BlueprintType)
class BLOCKS_API UVoxScatterDataAsset : public UDataAsset
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UStaticMesh>>Meshes; // Mesh pool for each iteration

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<int64,UInstancedStaticMeshComponent*> MeshComponents; // Transformations for each mesh instance cache

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVoxScatterData ScatterData; // Scatter data configuration

    bool bIsValid() const
    {
        bool empty = !Meshes.IsEmpty() && !Materials.IsEmpty();
        bool equ = Meshes.Num() == ScatterData.iterations;

        return empty && equ;
    }
};
