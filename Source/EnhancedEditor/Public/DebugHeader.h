#pragma once

#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

namespace DebugHeader
{
	static void Print(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMessageDialog(EAppMsgType::Type MessageType, const FString& Message,
	                                              bool bShowCancelButton = true)
	{
		if (bShowCancelButton)
		{
			FText MessageTitle = FText::FromString(TEXT("Warning"));

			return FMessageDialog::Open(MessageType, FText::FromString(Message), &MessageTitle);
		}
		return FMessageDialog::Open(MessageType, FText::FromString(Message));
	}

	static void ShowNotifyInfo(const FString& Message)
	{
		//Setup class to initialize a notification.
		FNotificationInfo Info(FText::FromString(Message));
		Info.bUseLargeFont = true;
		Info.FadeInDuration = 8.f;

		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Success);
	}
}
