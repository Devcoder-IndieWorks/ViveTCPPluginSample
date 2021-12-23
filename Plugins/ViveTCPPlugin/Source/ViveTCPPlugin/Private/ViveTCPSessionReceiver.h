// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FViveTCPSessionReceiver : public FRunnable
{
public:
    FViveTCPSessionReceiver( class UViveTCPSocketServer* InSocketServer, struct FViveTCPSession* InSession );
    ~FViveTCPSessionReceiver();

    virtual uint32 Run() override;
    virtual void Stop() override;

private:
    FThreadSafeBool Stopping;

    TWeakObjectPtr<class UViveTCPSocketServer> SocketServer;
    TSharedPtr<FRunnableThread> Thread;
    struct FViveTCPSession* Session;
};
