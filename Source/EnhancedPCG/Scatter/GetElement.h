// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GetElement.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UGetElement : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ENHANCEDPCG_API IGetElement
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
		
	virtual void GetMesh(TSubclassOf<UDataAsset>* DataAsset,const FName Category)
	{
	
	};
	

private:
	// TSubclassOf<UDataAsset>* DataAsset = nullptr;
	FRandomStream random;
	
};
