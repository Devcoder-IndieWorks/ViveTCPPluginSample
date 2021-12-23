// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FViveTCPSessionSender : public FRunnable
{
public:
    FViveTCPSessionSender( class UViveTCPSocketServer* InSocketServer, struct FViveTCPSession* InSession );
    ~FViveTCPSessionSender();

    virtual uint32 Run() override;
    virtual void Stop() override;

    void SendMessage( const FString& InMessage, const TArray<uint8>& InByteArray );

private:
    void PauseThread( bool InPause );

private:
    FThreadSafeBool Stopping;
    FThreadSafeBool Paused;

    TWeakObjectPtr<class UViveTCPSocketServer> SocketServer;
    TSharedPtr<FRunnableThread> Thread;
    struct FViveTCPSession* Session;

    TQueue<FString> MessageQueue;
    TQueue<TArray<uint8>> ByteArrayQueue;
};
