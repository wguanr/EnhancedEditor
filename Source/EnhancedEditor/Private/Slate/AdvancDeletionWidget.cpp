#include "Slate/AdvancDeletionWidget.h"

#include "DebugHeader.h"
#include "AssetRegistry/AssetData.h"
#include "ObjectTools.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
    StoredAssetData = InArgs._AssetDataToStore;

    ChildSlot
        [SNew(SBorder)
             .Padding(8.0f)
                 [SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)[SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("Delete Selected"))).OnClicked(this, &SAdvanceDeletionTab::OnDeleteSelectedClicked)] + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)[SNew(SButton).Text(FText::FromString(TEXT("Refresh"))).OnClicked(this, &SAdvanceDeletionTab::OnRefreshClicked)]]

                  + SVerticalBox::Slot()
                        .FillHeight(1.0f)
                            [SAssignNew(AssetListView, SListView<TSharedPtr<FAssetData>>)
                                 .ItemHeight(24.0f)
                                 .ListItemsSource(&StoredAssetData)
                                 .SelectionMode(ESelectionMode::Multi)
                                 .OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList)]]];
}

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(
    TSharedPtr<FAssetData> InItem,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
        [SNew(STextBlock).Text(FText::FromName(InItem.IsValid() ? InItem->AssetName : FName(TEXT("<Invalid>"))))];
}

FReply SAdvanceDeletionTab::OnDeleteSelectedClicked()
{
    if (!AssetListView.IsValid()) {
        return FReply::Handled();
    }

    TArray<TSharedPtr<FAssetData>> SelectedItems;
    AssetListView->GetSelectedItems(SelectedItems);

    if (SelectedItems.Num() == 0) {
        DebugHeader::ShowNotifyInfo(TEXT("未选择任何资源"));
        return FReply::Handled();
    }

    TArray<FAssetData> AssetsToDelete;
    AssetsToDelete.Reserve(SelectedItems.Num());
    for (const TSharedPtr<FAssetData>& Item : SelectedItems) {
        if (Item.IsValid()) {
            AssetsToDelete.Add(*Item.Get());
        }
    }

    int32 DeletedCount = ObjectTools::DeleteAssets(AssetsToDelete);
    if (DeletedCount > 0) {
        // 从列表中移除已删除项
        for (const FAssetData& Deleted : AssetsToDelete) {
            StoredAssetData.RemoveAllSwap([&Deleted](const TSharedPtr<FAssetData>& Ptr) {
                return Ptr.IsValid() && Ptr->ObjectPath == Deleted.ObjectPath;
            });
        }
        AssetListView->RequestListRefresh();
        DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("删除 %d 项"), DeletedCount));
    }
    else {
        DebugHeader::Print(TEXT("删除失败或被取消"), FColor::Red);
    }

    return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnRefreshClicked()
{
    // 仅刷新视图。若需重新拉取数据，应由外部重新构建本控件并传入新数据。
    if (AssetListView.IsValid()) {
        AssetListView->RequestListRefresh();
        DebugHeader::ShowNotifyInfo(TEXT("列表已刷新"));
    }
    return FReply::Handled();
}
