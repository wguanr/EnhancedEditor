#include "EnhancedAssetActionLib.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistryHelpers.h"
#include "EditorUtilityLibrary.h"

#define LOCTEXT_NAMESPACE "EnhancedAssetActionLib"


void UEnhancedAssetActionLib::FixMeshCollision(UStaticMesh* StaticMesh)
{
    // 设置网格体的碰撞选项为 BlockAll
    StaticMesh->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
    StaticMesh->GetBodySetup()->DefaultInstance.SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

    FText TransactionTest = LOCTEXT("StaticMeshEditorSetCollisionSectionFlag", "Staticmesh editor: Set Collision For section");

    FScopedTransaction Transaction(TransactionTest);

    FProperty* Property = UStaticMesh::StaticClass()->FindPropertyByName(UStaticMesh::GetSectionInfoMapName());

    StaticMesh->PreEditChange(Property);
    StaticMesh->Modify();

    for (int32 SectionIndex = 0; SectionIndex < StaticMesh->GetNumSections(0); ++SectionIndex) {
        FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(0, SectionIndex);
        Info.bEnableCollision = true;
        StaticMesh->GetSectionInfoMap().Set(0, SectionIndex, Info);
    }

    // 更新网格体以应用更改
    StaticMesh->MarkPackageDirty();
    StaticMesh->PostEditChange();
}

TArray<FAssetData> UEnhancedAssetActionLib::FilterThinMesh(TArray<FAssetData> InAssets, float Thin_limit, float Vol_limit, const FString& suffix)
{
    check(Thin_limit>0);
    
    TArray<FAssetData> FilteredAssets;
    for (FAssetData Asset : InAssets) {
        if (Asset.IsValid() && (Asset.AssetClass.IsEqual(UStaticMesh::StaticClass()->GetFName()))) {
            FString ApproxSize;
            if (Asset.GetTagValue(FName("ApproxSize"), ApproxSize)) {
                // Add filtering logic here if needed
                TArray<FString> ApproxSizeArray;
                ApproxSize.ParseIntoArray(ApproxSizeArray,TEXT("x"));

                TArray<int32> app_size;
                for (const FString& SizeStr : ApproxSizeArray) {
                    app_size.Add(FCString::Atoi(*SizeStr));
                }
                app_size.Sort();
                
                float portion_condition = (float)(app_size[0] +  app_size[1]) / (float)app_size.Last();
                float vol_condition = (float)(app_size[0] * app_size[1] * app_size.Last()) / Vol_limit;
                if (portion_condition > Thin_limit || vol_condition > 1) {
                    continue;
                }
                
                // add suffix
                if (!suffix.IsEmpty() && !Asset.AssetName.ToString().Contains(suffix)) {
                    UEditorUtilityLibrary::RenameAsset(Asset.GetAsset(), (Asset.AssetName.ToString() + TEXT("_") + suffix));
                    UE_LOG(LogTemp, Warning, TEXT("Filtered Asset: %s, ApproxSize portion is : %f"), *Asset.GetFullName(), portion_condition);
                }

                FilteredAssets.Add(Asset);

            }
        }
    }

    return FilteredAssets;
}

#undef LOCTEXT_NAMESPACE