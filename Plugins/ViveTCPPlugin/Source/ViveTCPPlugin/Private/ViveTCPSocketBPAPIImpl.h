// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "ViveTCPSocketBPAPI.h"
#include "ViveTCPSocketBPAPIImpl.generated.h"

UCLASS() 
class UViveTCPSocketBPAPIImpl : public UObject, public IViveTCPSocketBPAPI
{
    GENERATED_UCLASS_BODY()
public:
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void AddOnTCPConnectionDelegate( const FViveTCPConnectionDelegate& InDelegate ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void RemoveOnTCPConnectionDelegate( const FViveTCPConnectionDelegate& InDelegate ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void AddOnTCPDisconnectionDelegate( const FViveTCPDisconnectionDelegate& InDelegate ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void RemoveOnTCPDisconnectionDelegate( const FViveTCPDisconnectionDelegate& InDelegate ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void AddOnTCPReceiveDelegate( const FViveTCPReceiveDelegate& InDelegate ) override;
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void RemoveOnTCPReceiveDelegate( const FViveTCPReceiveDelegate& InDelegate ) override;

    //-------------------------------------------------------------------------

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void InitTCPSocket( FString& OutConnectionID, const FString& InIP, int32 InPort, int32 InWaitTimeMS, 
        EViveTCPReceiveFilterType InReceiveFilters = EViveTCPReceiveFilterType::E_SAB ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void CloseTCPSocket( const FString& InConnectionID ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void SendTo( const FString& InConnectionID, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak = true ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    bool IsTCPConnected( const FString& InConnectionID ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void ActivateMessageWrapping( const FString& InConnectionID, FString InHeader = TEXT( "([{UE4-Head}])" ), FString InFooter = TEXT( "([{UE4-Foot}])" ) ) override;

    UFUNCTION( BlueprintCallable, Category="ViveTCPSocket" )
    void DeactivateMessageWrapping( const FString& InConnectionID ) override;

private:
    class UViveTCPSocketObject* GetSocketObject( const FString& InConnectionID ) const;

private:
    FViveTCPConnectionDelegateGroup OnTCPConnectionDelegateGroup;
    FViveTCPDisconnectionDelegateGroup OnTCPDisconnectionDelegateGroup;
    FViveTCPReceiveDelegateGroup OnTCPReceiveDelegateGroup;

    //-------------------------------------------------------------------------

    UPROPERTY()
    TMap<FString, class UViveTCPSocketObject*> TCPSocketObjects;
};
