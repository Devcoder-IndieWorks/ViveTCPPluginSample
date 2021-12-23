// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Sockets.h"
#include "Networking/Public/Networking.h"
#include "UObject/Object.h"
#include "ViveTCPServerTypes.h"
#include "ViveTCPReceiveFilterType.h"
#include "ViveTCPSocketServer.generated.h"

UCLASS() 
class UViveTCPSocketServer : public UObject
{
    GENERATED_UCLASS_BODY()
public:
    void StartServer( const FViveIpAndPortData& InIpPortData, const FString& InIP, int32 InPort, 
        EViveTCPReceiveFilterType InReceiveFilter, const FString& InServerID );
    void StopServer();

    void StartSession( FViveTCPSession& InSession, EViveTCPReceiveFilterType InReceiveFilter );
    const FViveTCPSession* GetSession( const FString& InKey ) const;
    bool GetSessions( TMap<FString, FViveTCPSession>& OutSessions ) const;
    void RemoveSession( const FString& InKey );

    //-------------------------------------------------------------------------

    void SetConnectionCallback( TFunction<void( EViveSocketServerConnectionType, bool, FString, FString, FString )>&& InCallback );
    void SetReceiveCallback( TFunction<void( FString, FString, const TArray<uint8>&, FString )>&& InCallback );

    //-------------------------------------------------------------------------

    void ConnectionCallback( EViveSocketServerConnectionType InConnectionType, bool InSuccess, const FString& InMessage, const FString& InSessionID, const FString& InServerID );
    void ReceiveCallback( const FString& InSessionID, const FString& InMessage, const TArray<uint8>& InByteArray, const FString& InServerID );

    //-------------------------------------------------------------------------

    void ActivateMessageWrapping( const FString& InHeader, const FString& InFooter );
    void DeactivateMessageWrapping();
    void GetMessageWrapping( FString& OutHeader, FString& OutFooter, bool& OutUseWrapping );

    //-------------------------------------------------------------------------

    void SendAll( const TArray<FString>& InSessionIDs, const FString& InMessage, const TArray<uint8>& InByteArray );
    void SendTo( const FString& InSessionID, const FString& InMessage, const TArray<uint8>& InByteArray );

    //-------------------------------------------------------------------------

    void RegisterSessionEvent( const FString& InSessionID, class UViveSessionEvent*& OutEvent );
    void UnregisterSessionEvent( const FString& InSessionID );
    class UViveSessionEvent* GetRegisteredSessionEvent( const FString& InSessionID ) const;

    //-------------------------------------------------------------------------

    bool IsRunning() const;
    void SetRunning( bool InRunning );

    FString GetServerID() const;
    bool GetIpArray( TArray<int32>& OutIpArray ) const;
    FString GetIP() const;
    int32 GetPort() const;
    EViveTCPReceiveFilterType GetReceiveFilter() const;

private:
    void AddSession( const FViveTCPSession& InSession );

private:
    bool Running;
    FString ServerID;

    TArray<int32> IpArray;
    FString IP;
    int32 Port;

    bool UseMessageWrapping;
    FString MessageHeader;
    FString MessageFooter;

    EViveTCPReceiveFilterType ReceiveFilter;

    TMap<FString, FViveTCPSession> ClientSessions;

    //-------------------------------------------------------------------------

    TFunction<void( EViveSocketServerConnectionType, bool, FString, FString, FString )> OnConnection;
    TFunction<void( FString, FString, const TArray<uint8>&, FString )> OnReceive;

    //-------------------------------------------------------------------------

    TSharedPtr<class FViveTCPServerThread> TCPServerThread;

    //-------------------------------------------------------------------------

    UPROPERTY()
    TMap<FString, class UViveSessionEvent*> SessionEvents;
};
