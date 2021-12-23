// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "UObject/ObjectMacros.h"
#include "ViveTCPServerTypes.generated.h"

UENUM( BlueprintType ) 
enum class EViveSocketServerConnectionType : uint8
{
    E_Server UMETA(DisplayName="Server"),
    E_Client UMETA(DisplayName="Client")
};

struct FViveIpAndPortData
{
    bool Success;
    FString ErrorMessage;
    TArray<int32> IpArray;
    int32 Port;
};

struct FViveTCPSession
{
    FString IP;
    int32 Port = -1;
    FString SessionID;
    FString ServerID;

    //-------------------------------------------------------------------------

    TSharedPtr<class FRunnable> SendThread;
    TSharedPtr<class FRunnable> ReceiveThread;

    //-------------------------------------------------------------------------

    class FSocket* Socket = nullptr;
    class ISocketSubsystem* SocketSubsystem = nullptr;
};
