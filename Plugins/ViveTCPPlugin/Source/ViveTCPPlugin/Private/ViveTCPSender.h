// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FViveTCPSender : public FRunnable
{
public:
    FViveTCPSender( class UViveTCPSocketObject* InSocketObject );
    ~FViveTCPSender();

    virtual uint32 Run() override;
    virtual void Stop() override;

    void SendMessage( const FString& InMessage, const TArray<uint8>& InByteArray );

private:
    void SendBytes( const TArray<uint8>& InByteArray, int32& OutSent );
    void PauseThread( bool InPause );

private:
    FThreadSafeBool Stopping;
    FThreadSafeBool Paused;

    TWeakObjectPtr<class UViveTCPSocketObject> SocketObject;
    TSharedPtr<FRunnableThread> Thread;

    TQueue<FString> MessageQueue;
    TQueue<TArray<uint8>> ByteArrayQueue;
};
