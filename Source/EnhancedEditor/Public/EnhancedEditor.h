// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Delegates/Delegate.h"

class FEnhancedEditorModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
#pragma region ContentBrowserMenuEXtension

    void IniCBMenuExtension();
    void OnModulesChanged(FName InModuleName, EModuleChangeReason InChangeReason);

    TArray<FString> FolderPathSelected;

    FDelegateHandle ContentBrowserModulesChangedHandle;

    TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
    void AddCustomMenuEntry(class FMenuBuilder& MenuBuilder);
    void OnCleanUselessShit();
    void OnDeleteUnusedAssetsButtonCliked();
    void OnDeleteUnusedFoldersButtonCliked();

    //Slat
    void OnOpenSlatePannel();

    void FixupRedirectors();
#pragma endregion

#pragma region CustomEditorTab

    void RegisterAdvanceDeletionTab();

    TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

    TArray<TSharedPtr<FAssetData>> GetAssetData();
#pragma endregion

public:
#pragma region ProcessDataForAdvanceDeletionTab

    bool DeleteSingleAssetBySlate(const FAssetData& AssetDataToDelete);
    bool DeleteMultipleAssetsBySlate(const TArray<FAssetData>& AssetsDataToDelete);
#pragma endregion
};
