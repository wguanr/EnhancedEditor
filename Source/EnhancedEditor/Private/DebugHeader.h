#pragma once

#include "CoreMinimal.h"

class DebugHeader
{
public:
    static void Print(const FString& InMessage, const FColor& InColor = FColor::White);

    static void PrintLog(const FString& InMessage);

    static void ShowNotifyInfo(const FString& InMessage, float Duration = 3.0f);

    static EAppReturnType::Type ShowMessageDialog(EAppMsgType::Type MsgType, const FString& InMessage);
};
