// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FViveTCPConnection : public FRunnable
{
public:
    FViveTCPConnection( class UViveTCPSocketObject* InSocketObject );
    ~FViveTCPConnection();

    virtual uint32 Run() override;

private:
    TWeakObjectPtr<class UViveTCPSocketObject> SocketObject;
    TSharedPtr<FRunnableThread> Thread;
    class FSocket* Socket;
    class ISocketSubsystem* SocketSubsystem;
};
