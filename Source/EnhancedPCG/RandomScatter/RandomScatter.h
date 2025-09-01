// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "FScatterData.h"
#include "RandomScatter.generated.h"

UCLASS(Blueprintable)
class BLOCKS_API ARandomScatter : public AActor
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

public:
    ARandomScatter();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Generator")
    void ScatterActors(const AVolume* Bounds, const FString& MatFolderPath, const FString& MeshFolderPath);

    UPROPERTY()
    FScatterData ScatterData;

private:
    TObjectPtr<AActor> GenRandomActor(const TSubclassOf<AActor>& ActorClass, const FVector& Location, TArray<UMaterialInterface*> Material);

    void GetMaterialsFromFolder(const FString& MatFolderPath, TArray<UMaterialInstance*>& OutMaterials);

    void GetMeshFromFolder(const FString& MeshFolderPath, TArray<UStaticMesh*>& OutAssets);

    void GenRandomMeshComp(int CompNum, TArray<FVector> Location, TArray<UMaterialInstance*> Material, TArray<UStaticMesh*> StaticMeshes);

    void ReadJson();
};