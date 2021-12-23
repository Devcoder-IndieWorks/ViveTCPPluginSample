// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FViveTCPReceiver : public FRunnable
{
public:
    FViveTCPReceiver( class UViveTCPSocketObject* InSocketObject );
    ~FViveTCPReceiver();

    virtual uint32 Run() override;
    virtual void Stop() override;

private:
    FThreadSafeBool Stopping;

    TWeakObjectPtr<class UViveTCPSocketObject> SocketObject;
    TSharedPtr<FRunnableThread> Thread;
};
