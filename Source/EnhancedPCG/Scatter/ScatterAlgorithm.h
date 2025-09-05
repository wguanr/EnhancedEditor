#pragma once

#include "CoreMinimal.h"
#include "ScatterTypes.h"
#include "ScatterAlgorithm.generated.h"

UENUM(BlueprintType)
enum class EAnchorSelectionMode : uint8
{
    UniformRandom UMETA(DisplayName = "Uniform Random"),
    WeightedByFloatHandle UMETA(DisplayName = "Weighted By FloatHandle"),
    ByTagEqual UMETA(DisplayName = "By Tag Equal")
};

USTRUCT(BlueprintType)
struct FAnchorSelectionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAnchorSelectionMode Mode = EAnchorSelectionMode::UniformRandom;

    // 选取上一层锚点的比例（0..1），决定进入本层区域的父锚数量
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Ratio = 1.f;

    // 当 Mode=ByTagEqual 时使用
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredTag = NAME_None;
};

USTRUCT(BlueprintType)
struct FScatterAnchorRegionConfig
{
    GENERATED_BODY()

    // Prev.Bound 无效时的回退区域半长（XY）与半高（Z）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ExtentXY = FVector2D(300.f, 300.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HalfHeightZ = 200.f;

    // 若 Prev.Bound 有效，是否优先使用其 XY 外接盒；可追加膨胀（像素/世界单位）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPreferPrevBound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExpandBy = 0.f;
};

// 锚点选择器接口（可蓝图扩展）
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, Abstract)
class ENHANCEDPCG_API UScatterAnchorSelector : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, Category = "Scatter|Anchor")
    void Select(const TArray<FScatterContext> &InPrevContexts,
                const FAnchorSelectionConfig &Config,
                int32 NumNeeded,
                TArray<int32> &OutSelectedIndices,
                int32 Seed) const;
    virtual void Select_Implementation(const TArray<FScatterContext> &InPrevContexts,
                                       const FAnchorSelectionConfig &Config,
                                       int32 NumNeeded,
                                       TArray<int32> &OutSelectedIndices,
                                       int32 Seed) const;
};

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, Abstract)
class ENHANCEDPCG_API UScatterAlgorithm : public UObject
{
    GENERATED_BODY()
public:
    // 锚点选择与区域配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Anchor")
    TObjectPtr<UScatterAnchorSelector> AnchorSelector = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Anchor")
    FAnchorSelectionConfig AnchorSelectConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Anchor")
    FScatterAnchorRegionConfig AnchorRegion;

    // 统一的采样入口：返回二维采样点（在 Bounds2D 内）或直接生成 FScatterContext
    UFUNCTION(BlueprintNativeEvent, Category = "Scatter|Algorithm")
    void Generate(const FBox2D &Bounds2D,
                  const TArray<FScatterContext> &InPrevContexts,
                  const FScatterLayerConfig &Config,
                  TArray<FVector2D> &OutPoints2D,
                  int32 Seed) const;
    virtual void Generate_Implementation(const FBox2D &Bounds2D,
                                         const TArray<FScatterContext> &InPrevContexts,
                                         const FScatterLayerConfig &Config,
                                         TArray<FVector2D> &OutPoints2D,
                                         int32 Seed) const;

protected:
    // 由单个锚点生成二维区域（裁剪到 Bounds2D）
    UFUNCTION(BlueprintNativeEvent, Category = "Scatter|Anchor")
    void MakeRegionFromAnchor(const FBox2D &Bounds2D,
                              const FScatterContext &Prev,
                              const FScatterAnchorRegionConfig &RegionCfg,
                              FBox2D &OutRegion) const;
    virtual void MakeRegionFromAnchor_Implementation(const FBox2D &Bounds2D,
                                                     const FScatterContext &Prev,
                                                     const FScatterAnchorRegionConfig &RegionCfg,
                                                     FBox2D &OutRegion) const;

    // 在指定 Region 内采样并追加至 InOutPoints2D（不超过 Quota）
    UFUNCTION(BlueprintNativeEvent, Category = "Scatter|Algorithm")
    void AppendRegionSamples(const FBox2D &Region,
                             const FScatterContext &Prev,
                             const FScatterLayerConfig &Config,
                             int32 Quota,
                             TArray<FVector2D> &InOutPoints2D,
                             int32 Seed) const;
    virtual void AppendRegionSamples_Implementation(const FBox2D &Region,
                                                    const FScatterContext &Prev,
                                                    const FScatterLayerConfig &Config,
                                                    int32 Quota,
                                                    TArray<FVector2D> &InOutPoints2D,
                                                    int32 Seed) const;
};

// Poisson Disk 策略
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ENHANCEDPCG_API UScatterAlgorithm_PoissonDisk : public UScatterAlgorithm
{
    GENERATED_BODY()
public:
    virtual void AppendRegionSamples_Implementation(const FBox2D &Region,
                                                    const FScatterContext &Prev,
                                                    const FScatterLayerConfig &Config,
                                                    int32 Quota,
                                                    TArray<FVector2D> &InOutPoints2D,
                                                    int32 Seed) const override;
};

// Grid Jitter 策略
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ENHANCEDPCG_API UScatterAlgorithm_GridJitter : public UScatterAlgorithm
{
    GENERATED_BODY()
public:
    // 复用 Config.MinDistance 作为 CellSize 的基准
    virtual void AppendRegionSamples_Implementation(const FBox2D &Region,
                                                    const FScatterContext &Prev,
                                                    const FScatterLayerConfig &Config,
                                                    int32 Quota,
                                                    TArray<FVector2D> &InOutPoints2D,
                                                    int32 Seed) const override;
};

// Cluster Cores 策略
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ENHANCEDPCG_API UScatterAlgorithm_ClusterCores : public UScatterAlgorithm
{
    GENERATED_BODY()
public:
    virtual void AppendRegionSamples_Implementation(const FBox2D &Region,
                                                    const FScatterContext &Prev,
                                                    const FScatterLayerConfig &Config,
                                                    int32 Quota,
                                                    TArray<FVector2D> &InOutPoints2D,
                                                    int32 Seed) const override;
};
