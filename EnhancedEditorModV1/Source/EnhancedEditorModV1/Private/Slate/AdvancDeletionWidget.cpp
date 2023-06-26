#include "Slate/AdvancDeletionWidget.h"

#include "Widgets/Layout/SScrollBox.h"
#include "DebugHeader.h"
#include "../../Public/EnhancedEditorModV1.h"
#include "ObjectTools.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	// presets
	bCanSupportFocus = true;
	StoredAssetData = InArgs._AssetDataToStore; // get value from syntax: InArgs._[ArgumentName]
	CurrentCheckBox.Empty();
	AssetDatasChecked.Empty();

	FSlateFontInfo FontInfo = GetFontSytle();
	FontInfo.Size = 40;

	ChildSlot
	[
		// main slot
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advance Deletion")))
			.Font(FontInfo)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		// second slot for drop down
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]

		//thrid slot for a list view
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)

			+ SScrollBox::Slot()
			[
				ConstructListView()
			]
		]

		//forth slot for 3 button
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(FMargin(5.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .FillWidth(5.f)
			  .HAlign(HAlign_Fill)
			[
				ConstructSelectAllButton()
			]
			+ SHorizontalBox::Slot()
			  .FillWidth(5.f)
			  .HAlign(HAlign_Fill)
			[
				ConstructDeselectAllButton()
			]
			+ SHorizontalBox::Slot()
			  .FillWidth(5.f)
			  .HAlign(HAlign_Fill)
			[
				ConstructDeleteAllButton()
			]
		]

	];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructListView()
{
	AssetListView =
		SNew(SListView<TSharedPtr<FAssetData>>)
	.ItemHeight(24.f)
	.ListItemsSource(&StoredAssetData) // this is a TArray of above TSharedPtr<FAssetData>
	.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList);
	// A Pointer can convert to be a ref.
	return AssetListView.ToSharedRef();
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	CurrentCheckBox.Empty();
	AssetDatasChecked.Empty();

	if (AssetListView.IsValid())
	{
		AssetListView->RebuildList();
	}
}

#pragma region CustomRowWidgetView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
                                                                const TSharedRef<STableViewBase>& OwnerTable)
{
	//check,  in case to return a None ptr
	if (!AssetDataToDisplay.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}

	//main
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString DisplayClassName = AssetDataToDisplay->GetClass()->GetName();
	FSlateFontInfo AssetClassFont = GetFontSytle();
	AssetClassFont.Size = 13;
	FSlateFontInfo AssetFont = GetFontSytle();
	AssetFont.Size = 10;

	// define each row(SHorizontalBox= CheckBox + textbox + button) of the OwnerTable
	// return a ref of this row function as a widget
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(0.5f))
		[
			SNew(SHorizontalBox)

			// 1. check box to select assets
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Left)
			  .VAlign(VAlign_Center)
			  .FillWidth(.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)
			]

			//2. show the class name of the asset	
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Center)
			  .VAlign(VAlign_Center)
			  .FillWidth(.5f)
			[
				// SNew(STextBlock)
				// .Text(FText::FromString(DisplayClassName))
				ConstructTextBlock(DisplayClassName, AssetClassFont)
			]


			// primary text
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Left)
			  .VAlign(VAlign_Fill)
			  .FillWidth(.5f)
			[
				ConstructTextBlock(DisplayAssetName, AssetFont)
			]

			// 3. delete button
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Right)
			  .VAlign(VAlign_Fill)
			[
				ConstructButton(AssetDataToDisplay)
			]

		];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	CurrentCheckBox.Add(ConstructedCheckBox);
	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Checked:
		DebugHeader::Print(AssetData->AssetName.ToString(), FColor::Red);
		AssetDatasChecked.AddUnique(AssetData);
		break;
	case ECheckBoxState::Unchecked:
		DebugHeader::Print(AssetData->AssetName.ToString(), FColor::Green);
		if (AssetDatasChecked.Contains(AssetData))
		{
			AssetDatasChecked.Remove(AssetData);
		}

		break;
	case ECheckBoxState::Undetermined:
		break;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextBlock(const FString& TextContent,
                                                               const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructTextBlock =
		SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);
	return ConstructTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButton(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> Button =
		SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return Button;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FEnhancedEditorModV1Module& EnhancedEditorModV1 = FModuleManager::LoadModuleChecked<FEnhancedEditorModV1Module>(
		TEXT("EnhancedEditorModV1"));

	const bool bAssetDeleted = EnhancedEditorModV1.DeleteSingleAssetBySlate(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		//refresh the list
		if (StoredAssetData.Contains(ClickedAssetData))
		{
			StoredAssetData.Remove(ClickedAssetData);
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

#pragma endregion


#pragma region CustomAllButton
// construct delete all , select all and deselect all butt
TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> selButton =
		SNew(SButton)
	.Text(FText::FromString(TEXT("Select All")))
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnSelectButtonClicked);

	return selButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> deselButton =
		SNew(SButton)
	.Text(FText::FromString(TEXT("Deselect")))
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeselectButtonClicked);

	deselButton->SetContent(ConstructTextBlockForTabButton(TEXT("Deselect All")));

	return deselButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> delButton =
		SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked);

	delButton->SetContent(ConstructTextBlockForTabButton(TEXT("Delete All")));

	return delButton;
}

FReply SAdvanceDeletionTab::OnSelectButtonClicked()
{
	if (CurrentCheckBox.Num() == 0)
	{
		return FReply::Handled();
	}

	for (const TSharedPtr<SCheckBox>& CheckBox : CurrentCheckBox)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectButtonClicked()
{
	if (CurrentCheckBox.Num() == 0)
	{
		return FReply::Handled();
	}

	for (const TSharedPtr<SCheckBox>& CheckBox : CurrentCheckBox)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	RefreshAssetListView();

	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked()
{
	if (AssetDatasChecked.Num() == 0)
	{
		return FReply::Handled();
	}
	TArray<FAssetData> AssetDataToDelete;

	for (const TSharedPtr<FAssetData>& Data : AssetDatasChecked)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FEnhancedEditorModV1Module& EnhancedEditorModV1 = FModuleManager::LoadModuleChecked<FEnhancedEditorModV1Module>(
		TEXT("EnhancedEditorModV1"));

	bool bAssetsDeleted = EnhancedEditorModV1.DeleteMultipleAssetsBySlate(AssetDataToDelete);

	// update the list to avoid garbage data;
	if (bAssetsDeleted)
	{
		for (const TSharedPtr<FAssetData>& Data : AssetDatasChecked)
		{
			if (StoredAssetData.Contains(Data))
			{
				StoredAssetData.Remove(Data);
			}
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextBlockForTabButton(const FString& content)
{
	FSlateFontInfo ButtonText = GetFontSytle();
	ButtonText.Size = 15;
	return
		SNew(STextBlock)
	.Text(FText::FromString(content))
	.Font(ButtonText)
	.Justification(ETextJustify::Center);
}


#pragma endregion
