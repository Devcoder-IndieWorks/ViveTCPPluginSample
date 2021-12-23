// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ViveTCPReceiveFilterType.h"
#include "ViveTCPServerTypes.h"
#include "ViveTCPSocketServerBPAPI.generated.h"

DECLARE_DYNAMIC_DELEGATE_FiveParams( FViveTCPServerConnectionDelegate, EViveSocketServerConnectionType, InType, bool, InSuccess, FString, InMessage, FString, InSessionID, FString, InServerID );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams( FViveTCPServerConnectionDelegateGroup, EViveSocketServerConnectionType, InType, bool, InSuccess, FString, InMessage, FString, InSessionID, FString, InServerID );
DECLARE_DYNAMIC_DELEGATE_FourParams( FViveTCPServerReceiveDelegate, FString, InSessionID, FString, InMessage, const TArray<uint8>&, InByteArray, FString, InServerID );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FViveTCPServerReceiveDelegateGroup, FString, InSessionID, FString, InMessage, const TArray<uint8>&, InByteArray, FString, InServerID );

//-----------------------------------------------------------------------------

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class VIVETCPPLUGIN_API UViveTCPSocketServerBPAPI : public UInterface
{
    GENERATED_BODY()
};

//-----------------------------------------------------------------------------

class VIVETCPPLUGIN_API IViveTCPSocketServerBPAPI
{
    GENERATED_BODY()
public:
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void AddOnTCPServerConnectionDelegate( const FViveTCPServerConnectionDelegate& InDelegate ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void RemoveOnTCPServerConnectionDelegate( const FViveTCPServerConnectionDelegate& InDelegate ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void AddOnTCPServerReceiveDelegate( const FViveTCPServerReceiveDelegate& InDelegate ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void RemoveOnTCPServerReceiveDelegate( const FViveTCPServerReceiveDelegate& InDelegate ) {}

    //-------------------------------------------------------------------------

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void StartTCPSocketServer( FString& OutServerID, const FString& InIP = TEXT( "0.0.0.0" ), int32 InPort = 8888, 
        EViveTCPReceiveFilterType InReceiveFilter = EViveTCPReceiveFilterType::E_SAB, FString InCustomServerID = TEXT( "" ) ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void StopTCPSocketServer( const FString& InServerID ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void StopAllTCPSocketServer() {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void GetSocketServerSessionIDs( const FString& InServerID, TArray<FString>& OutSessionIDs ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void GetSocketServerSessionInfo( const FString& InSessionID, bool& OutSessionFound, FString& OutIP, int32& OutPort, FString& OutServerID ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void GetSocketServerSessionInfoByServerID( const FString& InServerID, const FString& InSessionID, bool& OutSessionFound, FString& OutIP, int32& OutPort ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void RemoveSessionAndCloseConnection( const FString& InServerID, const FString& InSessionID ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void ActivateMessageWrapping( const FString& InServerID, FString InHeader = TEXT( "([{UE4-Head}])" ), FString InFooter = TEXT( "([{UE4-Foot}])" ) ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void DeactivateMessageWrapping( const FString& InServerID ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void GetMessageWrapping( const FString& InServerID, FString& OutHeader, FString& OutFooter, bool& OutUseWrapping ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void SendAll( const FString& InServerID, const TArray<FString>& InSessionIDs, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = true ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void SendTo( const FString& InServerID, const FString& InSessionID, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = true ) {}

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void RegisterSessionEvent( const FString& InServerID, const FString& InSessionID, class UViveSessionEvent*& OutEvent ) {}
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    virtual void UnregisterSessionEvent( const FString& InServerID, const FString& InSessionID ) {}
};
