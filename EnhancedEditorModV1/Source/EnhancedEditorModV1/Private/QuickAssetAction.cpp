// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetAction.h"
#include "DebugHeader.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if(NumOfDuplicates<=0)
	{
		// Print(TEXT("Please enter a positive number of duplicates"), FColor::Red);
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please enter a positive number of duplicates"));
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
		DebugHeader::ShowNotifyInfo(TEXT("Success duplicated" + FString::FromInt(Counter) + " assets" ) );
	}
	
}

void UQuickAssetAction::AddPrefix()
{
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	UINT32 Counter = 0;
	for (UObject* SelectedAsset:SelectedAssets)
	{
		if (!SelectedAsset) continue;
		// this will return a ** pointer ** to the key value
		FString* PrefixFound = PrefixMap.Find(SelectedAsset->GetClass());
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Prefix doesnt exist for asset type :" + SelectedAsset->GetClass()->GetName()),FColor::Red);
			continue;
		}

		FString OldAssetName = SelectedAsset->GetName();
		// check the string using most FString functions
		if (OldAssetName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(TEXT("Prefix already exists for asset :"+ SelectedAsset->GetName()),FColor::Red);
			continue;
		}
		if (SelectedAsset->IsA<UMaterialInstanceConstant>())
		{
			OldAssetName.RemoveFromStart(TEXT("M_"));
			OldAssetName.RemoveFromEnd(TEXT("_inst"));
		}
		

		const FString NewAssetPath = *PrefixFound + OldAssetName;

		UEditorUtilityLibrary::RenameAsset(SelectedAsset, NewAssetPath);

		++Counter;
		
	}
	
	DebugHeader::ShowNotifyInfo(TEXT("I have renamed " + FString::FromInt(Counter) + " assets"));
	
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetData;

	FixupRedirectors();
	
	for(const FAssetData& AssetData:SelectedAssetData)
	{
		// remember to saved level and others first
		TArray<FString> ReferencesForAsset = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData.GetObjectPathString());

		if(ReferencesForAsset.Num()==0)
		{
			UnusedAssetData.Add(AssetData);
		}
	}

	if (UnusedAssetData.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("Find nothing Unused"),false);
		return;
	}

	// main
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetData,false);
	if(NumOfAssetsDeleted ==0) return;

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + "Assets"));
	
}

void UQuickAssetAction::FixupRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFix;
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjcetRedirector");

	TArray<FAssetData> OutAssetData;
	// the module reference the OutAssetData var to get the right assets
	AssetRegistryModule.Get().GetAssets(Filter,OutAssetData);

	for ( const FAssetData AssetData:OutAssetData)
	{
		if(UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(AssetData.GetAsset()))
		{
			RedirectorsToFix.Add(RedirectorToFix);
		}
		
	}

	
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFix);
}
