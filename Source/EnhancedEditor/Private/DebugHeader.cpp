#include "DebugHeader.h"

#include "Engine/Engine.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"

void DebugHeader::Print(const FString& InMessage, const FColor& InColor)
{
    if (GEngine) {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, InColor, InMessage);
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *InMessage);
}

void DebugHeader::PrintLog(const FString& InMessage)
{
    UE_LOG(LogTemp, Log, TEXT("%s"), *InMessage);
}

void DebugHeader::ShowNotifyInfo(const FString& InMessage, float Duration)
{
    FNotificationInfo Info(FText::FromString(InMessage));
    Info.ExpireDuration = Duration;
    Info.FadeOutDuration = 0.3f;
    Info.bUseSuccessFailIcons = false;

    TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
    if (Notification.IsValid()) {
        Notification->SetCompletionState(SNotificationItem::CS_Success);
    }
}

EAppReturnType::Type DebugHeader::ShowMessageDialog(EAppMsgType::Type MsgType, const FString& InMessage)
{
    return FMessageDialog::Open(MsgType, FText::FromString(InMessage));
}
