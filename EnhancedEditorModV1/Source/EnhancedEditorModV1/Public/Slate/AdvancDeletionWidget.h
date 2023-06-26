#pragma once

#include "Widgets/SCompoundWidget.h"


class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab)
		{
		}

		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetDataToStore)

	SLATE_END_ARGS()

public:
	// get the SLATE_ARGUMENT(...) , thanks to the maro.
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FAssetData>> StoredAssetData;
	TArray<TSharedPtr<SCheckBox>> CurrentCheckBox;

	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;
	void RefreshAssetListView();

#pragma region CustomRowWidgetView
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	                                           const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextBlock(const FString& TextContent, const FSlateFontInfo& FontToUse);
	FSlateFontInfo GetFontSytle() const { return FCoreStyle::Get().GetFontStyle("EmbossedText"); }

	TSharedRef<SButton> ConstructButton(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

#pragma endregion

	TArray<TSharedPtr<FAssetData>> AssetDatasChecked;

	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();
	TSharedRef<SButton> ConstructDeleteAllButton();

	FReply OnSelectButtonClicked();
	FReply OnDeselectButtonClicked();
	FReply OnDeleteButtonClicked();

	TSharedRef<STextBlock> ConstructTextBlockForTabButton(const FString& content);
};
