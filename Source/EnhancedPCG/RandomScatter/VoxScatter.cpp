// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxScatter.h"


// Sets default values
AVoxScatter::AVoxScatter()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    
    // VoxScatterSM = TArray<UStaticMesh*>();
    // VoxScatterMat = TArray<UMaterialInterface*>();
}

// Called when the game starts or when spawned
void AVoxScatter::BeginPlay()
{
    Super::BeginPlay();
    
}

// Called every frame
void AVoxScatter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AVoxScatter::VoxIterate()
{
}

void AVoxScatter::UpdateXFormsToDataAsset(const TArray<FTransform>& Xforms) const
{
}

