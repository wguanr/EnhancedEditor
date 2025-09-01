#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "FScatterData.generated.h"

// 散布层级枚举
UENUM(BlueprintType)
enum class EScatterLevel : uint8
{
    MainBuilding = 0 UMETA(DisplayName = "主体建筑"),
    SubBuilding = 1 UMETA(DisplayName = "附属构筑物"),
    SmallObject = 2 UMETA(DisplayName = "小物体")
};

// 散布配置结构 - 支持蓝图和JSON
USTRUCT(BlueprintType)
struct BLOCKS_API FLayerScatterConfig
{
    GENERATED_BODY()

    // 基础配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (ClampMin = "1"))
    int32 Num = 10;

    // 变换配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector ScalarDelta = FVector(0.2f, 0.2f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator RotationDelta = FRotator(15.0f, 180.0f, 15.0f);

    // 放置配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "50.0"))
    float MinDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D HeightOffsetRange = FVector2D(0.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bAlignToGround = true;

    // 碰撞配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bUseCollisionDetection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "10.0"))
    float CollisionRadius = 100.0f;

    // 射线检测距离
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Detection", meta = (ClampMin = "100.0"))
    float GroundTraceDistance = 10000.0f;

    FLayerScatterConfig()
    {
        Num = 10;
        ScalarDelta = FVector(0.2f, 0.2f, 0.2f);
        RotationDelta = FRotator(15.0f, 180.0f, 15.0f);
        MinDistance = 200.0f;
        HeightOffsetRange = FVector2D(0.0f, 50.0f);
        bAlignToGround = true;
        bUseCollisionDetection = true;
        CollisionRadius = 100.0f;
        GroundTraceDistance = 10000.0f;
    }

    // JSON读取函数
    void LoadFromJson(const FString& JsonFilePath, const FString& ConfigName)
    {
        FString JsonString;
        if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath)) {
            UE_LOG(LogTemp, Warning, TEXT("加载JSON文件失败: %s"), *JsonFilePath);
            return;
        }

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
        if (!FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
            UE_LOG(LogTemp, Error, TEXT("解析JSON文件失败"));
            return;
        }

        // 尝试读取特定层级的配置
        TSharedPtr<FJsonObject> LayerConfig = JsonObject->GetObjectField(ConfigName);
        if (!LayerConfig.IsValid()) {
            // 如果没有特定层级配置，使用通用配置
            UE_LOG(LogTemp, Warning, TEXT("未找到 %s 配置，使用通用配置"), *ConfigName);
            LoadFromJsonObject(JsonObject);
        }
        else {
            LoadFromJsonObject(LayerConfig);
        }
    }

private:
    void LoadFromJsonObject(TSharedPtr<FJsonObject> JsonObject)
    {
        if (!JsonObject.IsValid()) return;

        // 读取基础配置
        Num = JsonObject->GetIntegerField(TEXT("Num"));

        // 读取变换配置
        if (auto ScalarObj = JsonObject->GetObjectField(TEXT("ScalarDelta"))) {
            ScalarObj->TryGetNumberField(TEXT("X"), ScalarDelta.X);
            ScalarObj->TryGetNumberField(TEXT("Y"), ScalarDelta.Y);
            ScalarObj->TryGetNumberField(TEXT("Z"), ScalarDelta.Z);
        }

        if (auto RotationObj = JsonObject->GetObjectField(TEXT("RotationDelta"))) {
            RotationObj->TryGetNumberField(TEXT("Pitch"), RotationDelta.Pitch);
            RotationObj->TryGetNumberField(TEXT("Yaw"), RotationDelta.Yaw);
            RotationObj->TryGetNumberField(TEXT("Roll"), RotationDelta.Roll);
        }

        // 读取放置配置
        JsonObject->TryGetNumberField(TEXT("MinDistance"), MinDistance);
        JsonObject->TryGetBoolField(TEXT("bAlignToGround"), bAlignToGround);

        if (auto HeightObj = JsonObject->GetObjectField(TEXT("HeightOffsetRange"))) {
            HeightObj->TryGetNumberField(TEXT("X"), HeightOffsetRange.X);
            HeightObj->TryGetNumberField(TEXT("Y"), HeightOffsetRange.Y);
        }

        // 读取碰撞配置
        JsonObject->TryGetBoolField(TEXT("bUseCollisionDetection"), bUseCollisionDetection);
        JsonObject->TryGetNumberField(TEXT("CollisionRadius"), CollisionRadius);
        JsonObject->TryGetNumberField(TEXT("GroundTraceDistance"), GroundTraceDistance);
    }
};

// ===== 旧版本兼容 - 保持原有的FScatterData结构体 =====
USTRUCT(BlueprintType)
struct BLOCKS_API FScatterData
{
    GENERATED_BODY()

    // 基础散布配置 - 与JsonObjectConverter兼容
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    int32 Num = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FVector ScalarDelta = FVector(0.2f, 0.2f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    FRotator RotationDelta = FRotator(15.0f, 180.0f, 15.0f);

    // 扩展配置（可选）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float MinDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bAlignToGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bUseCollisionDetection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionRadius = 100.0f;

    // 材质和网格路径配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString MaterialFolderPath = TEXT("/Game/Materials");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString MeshFolderPath = TEXT("/Game/Meshes");

    FScatterData()
    {
        Num = 100;
        ScalarDelta = FVector(0.2f, 0.2f, 0.2f);
        RotationDelta = FRotator(15.0f, 180.0f, 15.0f);
        MinDistance = 200.0f;
        bAlignToGround = true;
        bUseCollisionDetection = true;
        CollisionRadius = 100.0f;
        MaterialFolderPath = TEXT("/Game/Materials");
        MeshFolderPath = TEXT("/Game/Meshes");
    }
};