// Copyright Epic Games, Inc. All Rights Reserved.

#include "Learn01.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "FLearn01Module"

void FLearn01Module::StartupModule()
{
	IniCBMenuExtension();
}

void FLearn01Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ContentBrowserMuneExtension

void FLearn01Module::IniCBMenuExtension()
{
	FContentBrowserModule& CBModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& CBMenuExtenders = 
		CBModule.GetAllPathViewContextMenuExtenders();

	// now we got an Delegate. then to add mem-func and params
	
	// FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	// CustomCBMenuDelegate.BindRaw(this,&FLearn01Module::CustomCBMenuExtender);
	// CBMenuExtenders.Add(CustomCBMenuDelegate);
	// above is equal to :

	CBMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this,&FLearn01Module::CustomCBMenuExtender));
}

TSharedRef<FExtender> FLearn01Module::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	// return a smart pointer! the new word is usually used in a smart pointer!
	TSharedRef<FExtender> MenuExtender(new FExtender());

	// Binding 01: Define the position for insert the entry
	if (SelectedPaths.Num()>0)
	{
		//, can find FName through Editor to check the "UI extension" in preference on!
		MenuExtender->AddMenuExtension(FName("Delete"),EExtensionHook::After,TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this,&FLearn01Module::AddCustomMenuEntry));
	}
	
	return MenuExtender;
}

// Binding 02: Defines the details
void FLearn01Module::AddCustomMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FLearn01Module::OnDeleteUnusedAssetsButtonCliked)
		);
}

// Binding 03: the real action------
void FLearn01Module::OnDeleteUnusedAssetsButtonCliked()
{
}

#pragma endregion 

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLearn01Module, Learn01)