#include "ScatterManager.h"

void AScatterManager::RunPipeline()
{
    if (bClearBeforeRun)
    {
        ClearAll();
    }

    // 将简化输入注入 DefaultAssetPool（若存在 Meshes）
    if (DefaultAssetPool.Entries.Num() == 0 && DefaultMeshes.Num() > 0)
    {
        for (const TSoftObjectPtr<UStaticMesh> &MeshSoft : DefaultMeshes)
        {
            FScatterAssetEntry Entry;
            Entry.Mesh = MeshSoft;
            Entry.CandidateMaterials = DefaultMaterials;
            Entry.Tag = DefaultTagForInjectedEntries;
            Entry.Weight = DefaultWeightForInjectedEntries;
            DefaultAssetPool.Entries.Add(Entry);
        }
    }

    TArray<FScatterContext> AccumulatedContexts;

    for (AScatterActorBase *Layer : ScatterLayers)
    {
        if (!IsValid(Layer))
        {
            continue;
        }
        // 若子层未配置 AssetPool，则注入默认池
        if (Layer->AssetPool.Entries.Num() == 0 && DefaultAssetPool.Entries.Num() > 0)
        {
            Layer->AssetPool = DefaultAssetPool;
        }
        Layer->AcceptPreviousContexts(AccumulatedContexts);
        Layer->ScatterThis();
        AccumulatedContexts.Append(Layer->GetOutputs().InstanceScatterContext);
    }
}

void AScatterManager::ClearAll()
{
    for (AScatterActorBase *Layer : ScatterLayers)
    {
        if (IsValid(Layer))
        {
            Layer->ClearPreviousScatter();
        }
    }
}
