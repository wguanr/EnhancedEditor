#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FAssetData;

class SAdvanceDeletionTab : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}
    SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetDataToStore)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TArray<TSharedPtr<FAssetData>> StoredAssetData;
    TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;

    TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> InItem, const TSharedRef<STableViewBase>& OwnerTable);

    FReply OnDeleteSelectedClicked();
    FReply OnRefreshClicked();
};
