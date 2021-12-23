// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ViveTCPReceiveFilterType.h"
#include "ViveTCPSocketBPAPI.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams( FViveTCPConnectionDelegate, bool, Success, FString, Message, FString, ConnectionID );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FViveTCPConnectionDelegateGroup, bool, Success, FString, Message, FString, ConnectionID );
DECLARE_DYNAMIC_DELEGATE( FViveTCPDisconnectionDelegate );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FViveTCPDisconnectionDelegateGroup );
DECLARE_DYNAMIC_DELEGATE_ThreeParams( FViveTCPReceiveDelegate, FString, Message, const TArray<uint8>&, ByteArray, FString, ConnectionID );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FViveTCPReceiveDelegateGroup, FString, Message, const TArray<uint8>&, ByteArray, FString, ConnectionID );

//-----------------------------------------------------------------------------

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class VIVETCPPLUGIN_API UViveTCPSocketBPAPI : public UInterface
{
    GENERATED_BODY()
};

//-----------------------------------------------------------------------------

class VIVETCPPLUGIN_API IViveTCPSocketBPAPI
{
    GENERATED_BODY()
public:
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void AddOnTCPConnectionDelegate( const FViveTCPConnectionDelegate& InDelegate ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void RemoveOnTCPConnectionDelegate( const FViveTCPConnectionDelegate& InDelegate ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void AddOnTCPDisconnectionDelegate( const FViveTCPDisconnectionDelegate& InDelegate ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void RemoveOnTCPDisconnectionDelegate( const FViveTCPDisconnectionDelegate& InDelegate ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void AddOnTCPReceiveDelegate( const FViveTCPReceiveDelegate& InDelegate ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void RemoveOnTCPReceiveDelegate( const FViveTCPReceiveDelegate& InDelegate ) {}

    //-------------------------------------------------------------------------

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void InitTCPSocket( FString& OutConnectionID, const FString& InIP, int32 InPort, int32 InWaitTimeMS, 
        EViveTCPReceiveFilterType InReceiveFilters = EViveTCPReceiveFilterType::E_SAB ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void CloseTCPSocket( const FString& InConnectionID ){}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void SendTo( const FString& InConnectionID, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = true ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual bool IsTCPConnected( const FString& InConnectionID ) { return false; }

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void ActivateMessageWrapping( const FString& InConnectionID, FString InHeader = TEXT( "([{UE4-Head}])" ), FString InFooter = TEXT( "([{UE4-Foot}])" ) ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    virtual void DeactivateMessageWrapping( const FString& InConnectionID ) {}
};
