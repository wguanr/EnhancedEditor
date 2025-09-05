#include "ScatterAlgorithm.h"
#include "Math/RandomStream.h"

void UScatterAlgorithm::Generate_Implementation(const FBox2D &Bounds2D,
                                                const TArray<FScatterContext> &InPrevContexts,
                                                const FScatterLayerConfig &Config,
                                                TArray<FVector2D> &OutPoints2D,
                                                int32 Seed) const
{
    OutPoints2D.Reset();
    if (!Bounds2D.bIsValid || Config.Num <= 0 || Config.MinDistance <= 0.f)
    {
        return;
    }

    // 选择上一层锚点
    TArray<int32> Selected;
    if (InPrevContexts.Num() > 0)
    {
        if (AnchorSelector)
        {
            AnchorSelector->Select(InPrevContexts, AnchorSelectConfig, Config.Num, Selected, Seed);
        }
        else
        {
            // 默认：按 Ratio 进行不放回随机选择
            FRandomStream Rng(Seed == 0 ? FMath::Rand() : Seed);
            const int32 TotalPrev = InPrevContexts.Num();
            const int32 Target = FMath::Clamp(FMath::RoundToInt(TotalPrev * FMath::Clamp(AnchorSelectConfig.Ratio, 0.f, 1.f)), 0, TotalPrev);
            TArray<int32> Indices;
            Indices.Reserve(TotalPrev);
            for (int32 i = 0; i < TotalPrev; ++i)
            {
                Indices.Add(i);
            }
            for (int32 i = 0; i < TotalPrev; ++i)
            {
                const int32 j = Rng.RandRange(i, TotalPrev - 1);
                Indices.Swap(i, j);
            }
            Indices.SetNum(Target);
            Selected = MoveTemp(Indices);
        }
    }

    if (Selected.Num() > 0)
    {
        // 每锚点配额，尽量平均分配
        int32 Remaining = Config.Num;
        for (int32 s = 0; s < Selected.Num() && Remaining > 0; ++s)
        {
            const int32 Idx = Selected[s];
            const FScatterContext &Prev = InPrevContexts[Idx];
            FBox2D Region;
            MakeRegionFromAnchor(Bounds2D, Prev, AnchorRegion, Region);
            if (!Region.bIsValid)
            {
                continue;
            }
            const int32 SlotsLeft = Selected.Num() - s;
            const int32 Quota = FMath::Max(1, Remaining / SlotsLeft);
            AppendRegionSamples(Region, Prev, Config, Quota, OutPoints2D, Seed + Idx);
            Remaining = FMath::Max(0, Config.Num - OutPoints2D.Num());
        }
        return;
    }

    // 无上一层锚点：在全局 Bounds2D 内采样
    FScatterContext DummyPrev;
    AppendRegionSamples(Bounds2D, DummyPrev, Config, Config.Num, OutPoints2D, Seed);
}

void UScatterAlgorithm_PoissonDisk::AppendRegionSamples_Implementation(const FBox2D &Region,
                                                                       const FScatterContext &Prev,
                                                                       const FScatterLayerConfig &Config,
                                                                       int32 Quota,
                                                                       TArray<FVector2D> &InOutPoints2D,
                                                                       int32 Seed) const
{
    if (!Region.bIsValid || Quota <= 0)
    {
        return;
    }
    const FVector2D Min = Region.Min;
    const FVector2D Max = Region.Max;
    const FVector2D Size = Max - Min;
    const float Cell = FMath::Max(1.f, Config.MinDistance * 0.7071f);
    const int32 NX = FMath::Max(1, FMath::FloorToInt(Size.X / Cell));
    const int32 NY = FMath::Max(1, FMath::FloorToInt(Size.Y / Cell));
    TArray<int32> Grid;
    Grid.Init(-1, NX * NY);
    auto ToIndex = [&](const FVector2D &P) -> int32
    { const int32 ix = FMath::Clamp(FMath::FloorToInt((P.X - Min.X) / Cell), 0, NX - 1); const int32 iy = FMath::Clamp(FMath::FloorToInt((P.Y - Min.Y) / Cell), 0, NY - 1); return iy * NX + ix; };
    const float MinDistSq = Config.MinDistance * Config.MinDistance;
    int32 Attempts = Quota * 25;
    while (Quota > 0 && Attempts-- > 0)
    {
        const FVector2D C(Min.X + FMath::FRand() * Size.X, Min.Y + FMath::FRand() * Size.Y);
        const int32 ci = ToIndex(C);
        bool bOk = true;
        const int32 cx = ci % NX;
        const int32 cy = ci / NX;
        for (int32 dy = -2; dy <= 2 && bOk; ++dy)
        {
            for (int32 dx = -2; dx <= 2 && bOk; ++dx)
            {
                const int32 nx = cx + dx;
                const int32 ny = cy + dy;
                if (nx < 0 || ny < 0 || nx >= NX || ny >= NY)
                {
                    continue;
                }
                const int32 gi = ny * NX + nx;
                const int32 pi = Grid[gi];
                if (pi >= 0 && FVector2D::DistSquared(InOutPoints2D[pi], C) < MinDistSq)
                {
                    bOk = false;
                }
            }
        }
        if (bOk)
        {
            Grid[ci] = InOutPoints2D.Add(C);
            --Quota;
        }
    }
}

void UScatterAlgorithm_GridJitter::AppendRegionSamples_Implementation(const FBox2D &Region,
                                                                      const FScatterContext &Prev,
                                                                      const FScatterLayerConfig &Config,
                                                                      int32 Quota,
                                                                      TArray<FVector2D> &InOutPoints2D,
                                                                      int32 Seed) const
{
    const float Cell = FMath::Max(10.f, Config.MinDistance);
    if (!Region.bIsValid || Cell <= KINDA_SMALL_NUMBER || Quota <= 0)
    {
        return;
    }
    const FVector2D Min = Region.Min;
    const FVector2D Max = Region.Max;
    const int32 NX = FMath::Max(1, FMath::FloorToInt((Max.X - Min.X) / Cell));
    const int32 NY = FMath::Max(1, FMath::FloorToInt((Max.Y - Min.Y) / Cell));
    int32 Added = 0;
    for (int32 ix = 0; ix < NX && Added < Quota; ++ix)
    {
        for (int32 iy = 0; iy < NY && Added < Quota; ++iy)
        {
            const float CX = Min.X + (ix + 0.5f) * Cell;
            const float CY = Min.Y + (iy + 0.5f) * Cell;
            const float JX = FMath::Clamp(FMath::FRandRange(-Config.Jitter, Config.Jitter), -1.f, 1.f) * Cell * 0.5f;
            const float JY = FMath::Clamp(FMath::FRandRange(-Config.Jitter, Config.Jitter), -1.f, 1.f) * Cell * 0.5f;
            InOutPoints2D.Add(FVector2D(CX + JX, CY + JY));
            ++Added;
        }
    }
}

void UScatterAlgorithm_ClusterCores::AppendRegionSamples_Implementation(const FBox2D &Region,
                                                                        const FScatterContext &Prev,
                                                                        const FScatterLayerConfig &Config,
                                                                        int32 Quota,
                                                                        TArray<FVector2D> &InOutPoints2D,
                                                                        int32 Seed) const
{
    if (!Region.bIsValid || Quota <= 0)
    {
        return;
    }
    const FVector2D Min = Region.Min;
    const FVector2D Max = Region.Max;
    const FVector2D Size = Max - Min;
    const int32 CoreCount = FMath::Clamp(Config.ClusterCoreCount, 1, FMath::Max(1, Quota));
    const float Influence = FMath::Clamp(Config.ClusterInfluenceRadius, 10.f, FMath::Max(Size.X, Size.Y));
    FRandomStream Rng(Seed == 0 ? FMath::Rand() : Seed);
    // 核心
    TArray<FVector2D> Cores;
    TArray<float> Strengths;
    int32 CoreAttempts = CoreCount * 12;
    while (Cores.Num() < CoreCount && CoreAttempts-- > 0)
    {
        const FVector2D C(Min.X + Rng.GetFraction() * Size.X, Min.Y + Rng.GetFraction() * Size.Y);
        bool bTooClose = false;
        for (const FVector2D &E : Cores)
        {
            if (FVector2D::Distance(E, C) < Influence * 0.8f)
            {
                bTooClose = true;
                break;
            }
        }
        if (!bTooClose)
        {
            Cores.Add(C);
            Strengths.Add(FMath::FRandRange(0.35f, 2.f));
        }
    }
    // 重要性采样
    const float MinDistSq = FMath::Square(FMath::Max(1.f, Config.MinDistance));
    int32 Attempts = Quota * 30;
    auto Gaussian = [&](FRandomStream &S)
    { float U=0.f; while(U<=0.f){U=S.GetFraction();} float V=S.GetFraction(); float M=FMath::Sqrt(-2.f*FMath::Loge(U)); float A=2.f*PI*V; return M*FMath::Cos(A); };
    while (Quota > 0 && Attempts-- > 0)
    {
        const bool bHasCores = Cores.Num() > 0;
        const int32 CoreIdx = bHasCores ? Rng.RandRange(0, Cores.Num() - 1) : 0;
        const FVector2D Core = bHasCores ? Cores[CoreIdx] : (Min + 0.5f * Size);
        const float CoreStrength = Strengths.Num() > 0 ? Strengths[CoreIdx] : 1.f;
        const float Radius = FMath::Abs(Gaussian(Rng)) * Influence * CoreStrength * 0.4f;
        const float Angle = 2.f * PI * Rng.GetFraction();
        FVector2D P = Core + FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));
        P.X = FMath::Clamp(P.X, Min.X, Max.X);
        P.Y = FMath::Clamp(P.Y, Min.Y, Max.Y);
        bool bOk = true;
        for (const FVector2D &E : InOutPoints2D)
        {
            if (FVector2D::DistSquared(E, P) < MinDistSq)
            {
                bOk = false;
                break;
            }
        }
        if (bOk)
        {
            InOutPoints2D.Add(P);
            --Quota;
        }
    }
}

void UScatterAnchorSelector::Select_Implementation(const TArray<FScatterContext> &InPrevContexts,
                                                   const FAnchorSelectionConfig &Config,
                                                   int32 NumNeeded,
                                                   TArray<int32> &OutSelectedIndices,
                                                   int32 Seed) const
{
    OutSelectedIndices.Reset();
    const int32 N = InPrevContexts.Num();
    if (N <= 0)
    {
        return;
    }
    FRandomStream Rng(Seed == 0 ? FMath::Rand() : Seed);

    // 预筛：ByTagEqual
    TArray<int32> Candidates;
    Candidates.Reserve(N);
    if (Config.Mode == EAnchorSelectionMode::ByTagEqual && !Config.RequiredTag.IsNone())
    {
        for (int32 i = 0; i < N; ++i)
        {
            if (InPrevContexts[i].tagHandle == Config.RequiredTag)
            {
                Candidates.Add(i);
            }
        }
    }
    else
    {
        Candidates.SetNumUninitialized(N);
        for (int32 i = 0; i < N; ++i)
        {
            Candidates[i] = i;
        }
    }
    if (Candidates.Num() == 0)
    {
        return;
    }

    const int32 Target = FMath::Clamp(FMath::RoundToInt(Candidates.Num() * FMath::Clamp(Config.Ratio, 0.f, 1.f)), 0, Candidates.Num());
    if (Target <= 0)
    {
        return;
    }

    if (Config.Mode == EAnchorSelectionMode::WeightedByFloatHandle)
    {
        // 轮盘赌选择（不放回）
        TArray<float> Weights;
        Weights.Reserve(Candidates.Num());
        for (int32 Idx : Candidates)
        {
            Weights.Add(FMath::Max(0.f, InPrevContexts[Idx].floatHandle));
        }
        for (int32 k = 0; k < Target && Candidates.Num() > 0; ++k)
        {
            float Sum = 0.f;
            for (float W : Weights)
            {
                Sum += W;
            }
            float T = Rng.GetFraction() * (Sum > 0.f ? Sum : (float)Candidates.Num());
            int32 Pick = 0;
            if (Sum > 0.f)
            {
                float Acc = 0.f;
                for (int32 i = 0; i < Weights.Num(); ++i)
                {
                    Acc += Weights[i];
                    if (Acc >= T)
                    {
                        Pick = i;
                        break;
                    }
                }
            }
            else
            {
                Pick = Rng.RandRange(0, Candidates.Num() - 1);
            }
            OutSelectedIndices.Add(Candidates[Pick]);
            Candidates.RemoveAtSwap(Pick);
            Weights.RemoveAtSwap(Pick);
        }
        return;
    }

    // UniformRandom（不放回洗牌）
    for (int32 i = 0; i < Candidates.Num(); ++i)
    {
        const int32 j = Rng.RandRange(i, Candidates.Num() - 1);
        Candidates.Swap(i, j);
    }
    Candidates.SetNum(Target);
    OutSelectedIndices = MoveTemp(Candidates);
}

void UScatterAlgorithm::MakeRegionFromAnchor_Implementation(const FBox2D &Bounds2D,
                                                            const FScatterContext &Prev,
                                                            const FScatterAnchorRegionConfig &RegionCfg,
                                                            FBox2D &OutRegion) const
{
    FVector2D Min2, Max2;
    if (RegionCfg.bPreferPrevBound && Prev.Bound.IsValid)
    {
        const FVector C = Prev.Bound.GetCenter();
        const FVector E = Prev.Bound.GetExtent();
        Min2 = FVector2D(C.X - E.X - RegionCfg.ExpandBy, C.Y - E.Y - RegionCfg.ExpandBy);
        Max2 = FVector2D(C.X + E.X + RegionCfg.ExpandBy, C.Y + E.Y + RegionCfg.ExpandBy);
    }
    else
    {
        const FVector A = Prev.Transform.GetLocation();
        Min2 = FVector2D(A.X - RegionCfg.ExtentXY.X, A.Y - RegionCfg.ExtentXY.Y);
        Max2 = FVector2D(A.X + RegionCfg.ExtentXY.X, A.Y + RegionCfg.ExtentXY.Y);
    }
    FBox2D Candidate(Min2, Max2);
    Candidate.bIsValid = true;
    // 裁剪到 Bounds2D
    const FVector2D CL(FMath::Max(Bounds2D.Min.X, Candidate.Min.X), FMath::Max(Bounds2D.Min.Y, Candidate.Min.Y));
    const FVector2D CU(FMath::Min(Bounds2D.Max.X, Candidate.Max.X), FMath::Min(Bounds2D.Max.Y, Candidate.Max.Y));
    OutRegion = FBox2D(CL, CU);
    OutRegion.bIsValid = (OutRegion.Min.X < OutRegion.Max.X && OutRegion.Min.Y < OutRegion.Max.Y);
}

void UScatterAlgorithm::AppendRegionSamples_Implementation(const FBox2D &Region,
                                                           const FScatterContext &Prev,
                                                           const FScatterLayerConfig &Config,
                                                           int32 Quota,
                                                           TArray<FVector2D> &InOutPoints2D,
                                                           int32 Seed) const
{
    // 默认实现：Poisson 区域采样
    if (!Region.bIsValid || Quota <= 0)
    {
        return;
    }
    const FVector2D Min = Region.Min;
    const FVector2D Max = Region.Max;
    const FVector2D Size = Max - Min;
    const float Cell = FMath::Max(1.f, Config.MinDistance * 0.7071f);
    const int32 NX = FMath::Max(1, FMath::FloorToInt(Size.X / Cell));
    const int32 NY = FMath::Max(1, FMath::FloorToInt(Size.Y / Cell));
    TArray<int32> Grid;
    Grid.Init(-1, NX * NY);
    auto ToIndex = [&](const FVector2D &P) -> int32
    { const int32 ix = FMath::Clamp(FMath::FloorToInt((P.X - Min.X) / Cell), 0, NX - 1); const int32 iy = FMath::Clamp(FMath::FloorToInt((P.Y - Min.Y) / Cell), 0, NY - 1); return iy * NX + ix; };
    const float MinDistSq = Config.MinDistance * Config.MinDistance;
    int32 Attempts = Quota * 25;
    while (Quota > 0 && Attempts-- > 0)
    {
        const FVector2D C(Min.X + FMath::FRand() * Size.X, Min.Y + FMath::FRand() * Size.Y);
        const int32 ci = ToIndex(C);
        bool bOk = true;
        const int32 cx = ci % NX;
        const int32 cy = ci / NX;
        for (int32 dy = -2; dy <= 2 && bOk; ++dy)
        {
            for (int32 dx = -2; dx <= 2 && bOk; ++dx)
            {
                const int32 nx = cx + dx;
                const int32 ny = cy + dy;
                if (nx < 0 || ny < 0 || nx >= NX || ny >= NY)
                {
                    continue;
                }
                const int32 gi = ny * NX + nx;
                const int32 pi = Grid[gi];
                if (pi >= 0 && FVector2D::DistSquared(InOutPoints2D[pi], C) < MinDistSq)
                {
                    bOk = false;
                }
            }
        }
        if (bOk)
        {
            Grid[ci] = InOutPoints2D.Add(C);
            --Quota;
        }
    }
}
