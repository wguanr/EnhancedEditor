// Copyright Epic Games, Inc. All Rights Reserved.

#include "../Public/EnhancedEditorModV1.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "EditorStyleSet.h"
#include "Slate/AdvancDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FEnhancedEditorModV1Module"

void FEnhancedEditorModV1Module::StartupModule()
{
	IniCBMenuExtension();
	RegisterAdvanceDeletionTab();
}

void FEnhancedEditorModV1Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ContentBrowserMuneExtension

void FEnhancedEditorModV1Module::IniCBMenuExtension()
{
	FContentBrowserModule& CBModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& CBMenuExtenders =
		CBModule.GetAllPathViewContextMenuExtenders();

	// now we got an Delegate. then to add mem-func and params

	// FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	// CustomCBMenuDelegate.BindRaw(this,&FEnhancedEditorModV1Module::CustomCBMenuExtender);
	// CBMenuExtenders.Add(CustomCBMenuDelegate);
	// above is equal to :

	CBMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FEnhancedEditorModV1Module::CustomCBMenuExtender));
}

TSharedRef<FExtender> FEnhancedEditorModV1Module::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	// return a smart pointer! the new word is usually used in a smart pointer!
	// this function is accept Paths like Dirpath but not asset, it will automaticlly add entry to the mune
	TSharedRef<FExtender> MenuExtender(new FExtender());
	// Binding 01: Define the position for insert the entry
	if (SelectedPaths.Num() > 0)
	{
		//, can find FName through Editor to check the "UI extension" in preference on!
		MenuExtender->AddMenuExtension(FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(),
		                               FMenuExtensionDelegate::CreateRaw(
			                               this, &FEnhancedEditorModV1Module::AddCustomMenuEntry));
		FolderPathSelected = SelectedPaths;
	}

	return MenuExtender;
}

// Binding 02: Defines the details
void FEnhancedEditorModV1Module::AddCustomMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Clean Useless Shit")),
		FText::FromString(TEXT("Safely delete all unused assets under folder,Delete Empty Folders")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FEnhancedEditorModV1Module::OnCleanUselessShit)
	);
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advanecd Deletion")),
		FText::FromString(TEXT("Custom Slate Pannel: Custom Slate Pannel")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FEnhancedEditorModV1Module::OnOpenSlatePannel)
	);
}

void FEnhancedEditorModV1Module::OnCleanUselessShit()
{
	// OnDeleteUnusedAssetsButtonCliked();
	// OnDeleteUnusedFoldersButtonCliked();

	///////

	if (FolderPathSelected.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("You can just chose one folder"));
		return;
	}

	TArray<FString> ShitPathInFolder = UEditorAssetLibrary::ListAssets(FolderPathSelected[0], true, true);
	FixupRedirectors();
	if (ShitPathInFolder.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("There is a empty folder"));
		return;
	}
	///////


	TArray<FAssetData> ShitAssetsDataArr;
	TArray<FString> ShitFolderArr;
	uint32 Counter = 0;
	for (const FString& ShitPath : ShitPathInFolder)
	{
		if (ShitPath.Contains(TEXT("Developers")) ||
			ShitPath.Contains(TEXT("Collections")) ||
			ShitPath.Contains(TEXT("__ExternalActors__")) ||
			ShitPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (UEditorAssetLibrary::DoesDirectoryExist(ShitPath) && !
			UEditorAssetLibrary::DoesDirectoryHaveAssets(ShitPath))
		{
			ShitFolderArr.Add(ShitPath);
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(ShitPath))
		{
			continue;
		}
		TArray<FString> Referencers = UEditorAssetLibrary::FindPackageReferencersForAsset(ShitPath);
		if (Referencers.Num() == 0)
		{
			FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(ShitPath);
			ShitAssetsDataArr.Add(UnusedAssetData);
		}
	}
	ObjectTools::DeleteAssets(ShitAssetsDataArr, true);

	for (const FString& ShitFolder : ShitFolderArr)
	{
		UEditorAssetLibrary::DeleteDirectory(ShitFolder)
			? Counter++
			: DebugHeader::Print(TEXT("Delete Failed"), FColor::Red);
	}
}


// Binding 03: the real action------
void FEnhancedEditorModV1Module::OnDeleteUnusedAssetsButtonCliked()
{
	// if the user give unexpected path ,then return the function.
	if (FolderPathSelected.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("You can just chose one folder"));
		return;
	}

	TArray<FString> AssetsPathInFolder = UEditorAssetLibrary::ListAssets(FolderPathSelected[0]);

	if (AssetsPathInFolder.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("There is a empty folder"));
		return;
	}

	const EAppReturnType::Type ConfirmedResult = DebugHeader::ShowMessageDialog(
		EAppMsgType::YesNo,TEXT("Are you sure to go?"));
	if (ConfirmedResult == EAppReturnType::No)
	{
		return;
	}

	FixupRedirectors();

	TArray<FAssetData> UnusedAssetsDataArr;

	for (const FString& AssetPath : AssetsPathInFolder)
	{
		if (AssetPath.Contains(TEXT("Developers")) ||
			AssetPath.Contains(TEXT("Collections")) ||
			AssetPath.Contains(TEXT("__ExternalActors__")) ||
			AssetPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
		{
			continue;
		}

		TArray<FString> Referencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath);
		if (Referencers.Num() == 0)
		{
			FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPath);
			UnusedAssetsDataArr.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArr.Num() > 0)
	{
		// use objtool to delete all assets array at great effficiency
		ObjectTools::DeleteAssets(UnusedAssetsDataArr);
	}
	else
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("All unused assets have been cleared"));
	}
}

void FEnhancedEditorModV1Module::OnDeleteUnusedFoldersButtonCliked()
{
	FixupRedirectors();

	if (FolderPathSelected.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("You can just chose one folder"));
		return;
	}

	TArray<FString> AssetsPathInFolder = UEditorAssetLibrary::ListAssets(FolderPathSelected[0], true, true);


	TArray<FString> EmptyFolderPath;
	uint32 Counter = 0;
	for (const FString& FolderPath : AssetsPathInFolder)
	{
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (UEditorAssetLibrary::DoesDirectoryExist(FolderPath) && !
			UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			UEditorAssetLibrary::DeleteDirectory(FolderPath)
				? ++Counter
				: DebugHeader::Print(TEXT("Faild to delete dir: ") + FolderPath, FColor::Red);
		}
	}

	DebugHeader::ShowNotifyInfo(TEXT("Succesfull to clear "+FString::FromInt(Counter)+" folders"));
}

void FEnhancedEditorModV1Module::OnOpenSlatePannel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FEnhancedEditorModV1Module::FixupRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFix;

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	// Learning Notes:
	// get class name : " ObjectRedirector" by 2 ways from 5.1
	// "/Script/[ProjectName].[EnumName]"
	// GetPathNameSafe(UClass::TryFindTypeSlow<UEnum>("EnumName"))
	Filter.ClassPaths.AddUnique(FTopLevelAssetPath("/Script/CoreUObject.ObjectRedirector"));

	// the module reference the OutAssetData var to get the right assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));
	TArray<FAssetData> OutAssetData;
	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);

	for (const FAssetData AssetData : OutAssetData)
	{
		// turn UObjcet* to UObjcetRedirector* using cast
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(AssetData.GetAsset()))
		{
			RedirectorsToFix.Add(RedirectorToFix);
		}
	}

	// Fix all by func FixupReferencers()
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<
		FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFix);
}


#pragma endregion

#pragma region CustomEditorTab

void FEnhancedEditorModV1Module::RegisterAdvanceDeletionTab()
{
	// what's the diffrence between LOCTEXT and FText?
	// LOCTEXT is a macro that will be replaced by a FText object at compile time.
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this,
		                                                  &FEnhancedEditorModV1Module::OnSpawnAdvancedDeletionTab))
	                        .SetDisplayName(LOCTEXT("AdvancedDeletionTabTitle", "Advanced Deletion"))
	                        .SetTooltipText(LOCTEXT("AdvancedDeletionTooltipText", "Open the Advanced Deletion tab"))
	                        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.PlacementBrowser"));
	// FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"),
	// 	FOnSpawnTab::CreateRaw(this,&FEnhancedEditorModV1Module::OnSpawnAdvancedDeletionTab));
}

TSharedRef<SDockTab> FEnhancedEditorModV1Module::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// there are several types of ue tabs in TabRole;
	// include slate c++ head file , using slate syntax to instance a tab stuff.
	return
		SNew(SDockTab).TabRole(NomadTab)
		[
			SNew(SAdvanceDeletionTab)
			.AssetDataToStore(GetAssetData())

		];
}

TArray<TSharedPtr<FAssetData>> FEnhancedEditorModV1Module::GetAssetData()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetData;

	TArray<FString> AssetsPathInFolder = UEditorAssetLibrary::ListAssets(FolderPathSelected[0]);

	for (const FString& AssetPath : AssetsPathInFolder)
	{
		if (AssetPath.Contains(TEXT("Developers")) ||
			AssetPath.Contains(TEXT("Collections")) ||
			AssetPath.Contains(TEXT("__ExternalActors__")) ||
			AssetPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
		{
			continue;
		}

		// TSharedPtr<FAssetData> AssetData = MakeShareable(new FAssetData(UEditorAssetLibrary::FindAssetData(AssetPath)));
		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPath);
		AvailableAssetData.Add(MakeShared<FAssetData>(Data));
	}

	return AvailableAssetData;
}


#pragma endregion

#pragma region ProcessDataForAdvanceDeletionTab

bool FEnhancedEditorModV1Module::DeleteSingleAssetBySlate(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> Assets;
	Assets.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(Assets) > 0)
	{
		return true;
	}


	return false;
}

bool FEnhancedEditorModV1Module::DeleteMultipleAssetsBySlate(const TArray<FAssetData>& AssetsDataToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsDataToDelete) > 0)
	{
		return true;
	}
	return false;
}
#pragma endregion

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEnhancedEditorModV1Module, EnhancedEditorModV1)
