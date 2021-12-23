// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "ViveTCPSocketServerBPAPI.h"
#include "ViveTCPSocketServerBPAPIImpl.generated.h"

UCLASS() 
class UViveTCPSocketServerBPAPIImpl : public UObject, public IViveTCPSocketServerBPAPI
{
    GENERATED_UCLASS_BODY()
public:
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void AddOnTCPServerConnectionDelegate( const FViveTCPServerConnectionDelegate& InDelegate ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void RemoveOnTCPServerConnectionDelegate( const FViveTCPServerConnectionDelegate& InDelegate ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void AddOnTCPServerReceiveDelegate( const FViveTCPServerReceiveDelegate& InDelegate ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void RemoveOnTCPServerReceiveDelegate( const FViveTCPServerReceiveDelegate& InDelegate ) override;

    //-------------------------------------------------------------------------

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void StartTCPSocketServer( FString& OutServerID, const FString& InIP = TEXT( "0,0,0,0" ), int32 InPort = 8888, 
        EViveTCPReceiveFilterType InReceiveFilter = EViveTCPReceiveFilterType::E_SAB, FString InCustomServerID = TEXT( "" ) ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void StopTCPSocketServer( const FString& InServerID ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void StopAllTCPSocketServer() override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void GetSocketServerSessionIDs( const FString& InServerID, TArray<FString>& OutSessionIDs ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void GetSocketServerSessionInfo( const FString& InSessionID, bool& OutSessionFound, FString& OutIP, int32& OutPort, FString& OutServerID ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void GetSocketServerSessionInfoByServerID( const FString& InServerID, const FString& InSessionID, bool& OutSessionFound, FString& OutIP, int32& OutPort ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void RemoveSessionAndCloseConnection( const FString& InServerID, const FString& InSessionID ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void ActivateMessageWrapping( const FString& InServerID, FString InHeader = TEXT( "([{UE4-Head}])" ), FString InFooter = TEXT( "([{UE4-Foot}])" ) ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void DeactivateMessageWrapping( const FString& InServerID ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void GetMessageWrapping( const FString& InServerID, FString& OutHeader, FString& OutFooter, bool& OutUseWrapping ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void SendAll( const FString& InServerID, const TArray<FString>& InSessionIDs, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = true ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void SendTo( const FString& InServerID, const FString& InSessionID, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = true ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void RegisterSessionEvent( const FString& InServerID, const FString& InSessionID, class UViveSessionEvent*& OutEvent ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void UnregisterSessionEvent( const FString& InServerID, const FString& InSessionID ) override;

private:
    class UViveTCPSocketServer* GetSocketServer( const FString& InServerID ) const;
    FViveIpAndPortData CheckIpAndPort( const FString& InIP, int32 InPort );

private:
    FViveTCPServerConnectionDelegateGroup OnTCPServerConnectionDelegateGroup;
    FViveTCPServerReceiveDelegateGroup OnTCPServerReceiveDelegateGroup;

    FString LastTCPSocketServerID;

    //-------------------------------------------------------------------------

    UPROPERTY()
    TMap<FString, class UViveTCPSocketServer*> TCPSocketServers;
};
