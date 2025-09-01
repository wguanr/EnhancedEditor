// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxScatterDataAsset.h"
#include "VoxScatter.generated.h"


UCLASS(Blueprintable)
class BLOCKS_API AVoxScatter : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AVoxScatter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSoftObjectPtr<UVoxScatterDataAsset> VoxScatterDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UVoxScatterDataAsset* ScatterData;

    UFUNCTION(CallInEditor, BlueprintCallable)
    void VoxIterate();

    
private:
    TArray<UStaticMesh*> VoxScatterSM;
    TArray<UMaterialInterface*> VoxScatterMat;

    
    TArray<FTransform> VoxScatterXforms;
    void UpdateXFormsToDataAsset(const TArray<FTransform>& Xforms) const;
};