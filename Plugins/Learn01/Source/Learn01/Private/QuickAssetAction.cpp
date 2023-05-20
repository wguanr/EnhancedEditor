// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetAction.h"
#include "DebugHeader.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"


void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if(NumOfDuplicates<=0)
	{
		// Print(TEXT("Please enter a positive number of duplicates"), FColor::Red);
		ShowMessageDialog(EAppMsgType::Ok, TEXT("Please enter a positive number of duplicates"));
		return;
	}

	TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();
	UINT32 Counter = 0;

	for (const FAssetData& SelectAssetData:SelectedAssetData)
	{
		for(int32 i =0; i<NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectAssetData.GetSoftObjectPath().ToString();
			const FString NewDuplicatedName = SelectAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i+1);
			const FString NewPathName = FPaths::Combine(SelectAssetData.PackagePath.ToString(), NewDuplicatedName);

			if(UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}
	
	if (Counter>0)
	{
		ShowNotifyInfo(TEXT("Success duplicated" + FString::FromInt(Counter) + " assets" ) );
	}
	
}
