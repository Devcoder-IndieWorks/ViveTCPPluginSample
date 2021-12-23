// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Sockets.h"
#include "Networking/Public/Networking.h"
#include "UObject/Object.h"
#include "ViveTCPReceiveFilterType.h"
#include "ViveTCPSocketObject.generated.h"

UCLASS() 
class UViveTCPSocketObject : public UObject
{
    GENERATED_UCLASS_BODY()
public:
    void Setup( const FString& InIP, int32 InPort, const FString& InConnectionID, int32 InWaitTimeMS = 100, 
        EViveTCPReceiveFilterType InReceiveFilter = EViveTCPReceiveFilterType::E_SAB );
    void Connection();
    void Disconnection();
    void StartSender();
    void StartReceiver();

    //-------------------------------------------------------------------------

    void SetConnectionCallback( TFunction<void( bool, const FString&, const FString& )>&& InCallback );
    void SetDisconnectionCallback( TFunction<void()>&& InCallback );
    void SetReceiveCallback( TFunction<void( const FString&, const TArray<uint8>&, const FString& )>&& InCallback );

    //-------------------------------------------------------------------------

    void ConnectionCallback( bool InSuccess, const FString& InMessage, const FString& InConnectionID );
    void DisconnectionCallback();
    void ReceiveCallback( const FString& InMessage, const TArray<uint8>& InByteArray, const FString& InConnectionID );

    //-------------------------------------------------------------------------

    void Send( const FString& InMessage, const TArray<uint8>& InByteArray );
    int32 Send( const TArray<uint8>& InByteArray );

    bool Receive( uint8* OutData, int32 InBufferSize, int32& OutBytesRead );

    bool Wait( const FTimespan& InWaitTime );
    bool HasPendingData( uint32& OutPendingDataSize );

    //-------------------------------------------------------------------------

    void ActivateMessageWrapping( const FString& InHeader, const FString& InFooter );
    void DeactivateMessageWrapping();
    void GetMessageWrapping( FString& OutHeader, FString& OutFooter, bool& OutUseWrapping );

    bool IsRunning() const;
    void SetRunning( bool InRunning );

    FSocket* GetSocket() const;
    void SetSocket( FSocket* InSocket );

    ISocketSubsystem* GetSocketSubsystem() const;
    void SetSocketSubsystem( ISocketSubsystem* InSocketSubsystem );

    FString GetConnectionID() const;
    FString GetIP() const;
    int32 GetPort() const;
    int32 GetWaitTimeMS() const;
    EViveTCPReceiveFilterType GetReceiveFilter() const;
    ESocketConnectionState GetConnectionState() const;

private:
    bool Running;
    FString ConnectionID;
    FString IP;
    int32 Port;
    int32 WaitTimeMS;

    bool UseMessageWrapping;
    FString MessageHeader;
    FString MessageFooter;

    EViveTCPReceiveFilterType ReceiveFilter;

    //-------------------------------------------------------------------------

    TFunction<void( bool, const FString&, const FString& )> OnConnection;
    TFunction<void()> OnDisconnection;
    TFunction<void( const FString&, const TArray<uint8>&, const FString& )> OnReceive;

    //-------------------------------------------------------------------------

    FSocket* Socket;
    ISocketSubsystem* SocketSubsystem;

    //-------------------------------------------------------------------------

    TSharedPtr<class FViveTCPConnection> TCPConnection;
    TSharedPtr<class FViveTCPSender> TCPSender;
    TSharedPtr<class FViveTCPReceiver> TCPReceiver;
};
