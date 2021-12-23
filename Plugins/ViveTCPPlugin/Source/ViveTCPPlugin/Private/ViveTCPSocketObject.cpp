#include "ViveTCPSocketObject.h"
#include "ViveTCPConnection.h"
#include "ViveTCPReceiver.h"
#include "ViveTCPSender.h"
#include "ViveLog.h"

UViveTCPSocketObject::UViveTCPSocketObject( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
    Running = false;
    Port = 0;
    WaitTimeMS = 0;
    UseMessageWrapping = false;
    ReceiveFilter = EViveTCPReceiveFilterType::E_SAB;
    Socket = nullptr;
    SocketSubsystem = nullptr;
}

void UViveTCPSocketObject::Setup( const FString& InIP, int32 InPort, const FString& InConnectionID, int32 InWaitTimeMS, 
    EViveTCPReceiveFilterType InReceiveFilter )
{
    IP = InIP;
    Port = InPort;
    ConnectionID = InConnectionID;
    WaitTimeMS = InWaitTimeMS;
    ReceiveFilter = InReceiveFilter;
}

void UViveTCPSocketObject::Connection()
{
    if ( !TCPConnection.IsValid() )
        TCPConnection = MakeShared<FViveTCPConnection>( this );
}

void UViveTCPSocketObject::Disconnection()
{
    if ( Running  ) {
        Running = false;

        if ( TCPSender.IsValid() ) {
            TCPSender->Stop();
            TCPSender = nullptr;
        }

        if ( TCPReceiver.IsValid() ) {
            TCPReceiver->Stop();
            TCPReceiver = nullptr;
        }

        if ( TCPConnection.IsValid() )
            TCPConnection = nullptr;

        Socket = nullptr;
        SocketSubsystem = nullptr;

        TWeakObjectPtr<UViveTCPSocketObject> thisPtr = this;
        AsyncTask( ENamedThreads::GameThread, [thisPtr]{
            if ( thisPtr.IsValid() )
                thisPtr->DisconnectionCallback();
        } );
    }
}

void UViveTCPSocketObject::StartSender()
{
    if ( !TCPSender.IsValid() )
        TCPSender = MakeShared<FViveTCPSender>( this );
}

void UViveTCPSocketObject::StartReceiver()
{
    if ( !TCPReceiver.IsValid() )
        TCPReceiver = MakeShared<FViveTCPReceiver>( this );
}

void UViveTCPSocketObject::SetConnectionCallback( TFunction<void( bool, const FString&, const FString& )>&& InCallback )
{
    OnConnection = MoveTemp( InCallback );
}

void UViveTCPSocketObject::SetDisconnectionCallback( TFunction<void()>&& InCallback )
{
    OnDisconnection = MoveTemp( InCallback );
}

void UViveTCPSocketObject::SetReceiveCallback( TFunction<void( const FString&, const TArray<uint8>&, const FString& )>&& InCallback )
{
    OnReceive = MoveTemp( InCallback );
}

void UViveTCPSocketObject::ConnectionCallback( bool InSuccess, const FString& InMessage, const FString& InConnectionID )
{
    if ( OnConnection )
        OnConnection( InSuccess, InMessage, InConnectionID );
}

void UViveTCPSocketObject::DisconnectionCallback()
{
    if ( OnDisconnection )
        OnDisconnection();
}

void UViveTCPSocketObject::ReceiveCallback( const FString& InMessage, const TArray<uint8>& InByteArray, const FString& InConnectionID )
{
    if ( OnReceive )
        OnReceive( InMessage, InByteArray, InConnectionID );
}

void UViveTCPSocketObject::Send( const FString& InMessage, const TArray<uint8>& InByteArray )
{
    if ( TCPSender.IsValid() )
        TCPSender->SendMessage( InMessage, InByteArray );
}

int32 UViveTCPSocketObject::Send( const TArray<uint8>& InByteArray )
{
    int32 bytesSent = 0;
    if ( ensure( Socket != nullptr ) )
        Socket->Send( InByteArray.GetData(), InByteArray.Num(), bytesSent );

    return bytesSent;
}

bool UViveTCPSocketObject::Receive( uint8* OutData, int32 InBufferSize, int32& OutBytesRead )
{
    OutBytesRead = 0;
    return ensure( Socket != nullptr ) 
               ? Socket->Recv( OutData, InBufferSize, OutBytesRead ) 
               : false;
}

bool UViveTCPSocketObject::Wait( const FTimespan& InWaitTime )
{
    return ensure( Socket != nullptr ) 
               ? Socket->Wait( ESocketWaitConditions::WaitForReadOrWrite, InWaitTime )
               : false;
}

bool UViveTCPSocketObject::HasPendingData( uint32& OutPendingDataSize )
{
    OutPendingDataSize = 0;
    return ensure( Socket != nullptr ) 
               ? Socket->HasPendingData( OutPendingDataSize ) 
               : false;
}

void UViveTCPSocketObject::ActivateMessageWrapping( const FString& InHeader, const FString& InFooter )
{
    MessageHeader = InHeader;
    MessageFooter = InFooter;
    UseMessageWrapping = true;
}

void UViveTCPSocketObject::DeactivateMessageWrapping()
{
    UseMessageWrapping = false;
}

void UViveTCPSocketObject::GetMessageWrapping( FString& OutHeader, FString& OutFooter, bool& OutUseWrapping )
{
    OutHeader = MessageHeader;
    OutFooter = MessageFooter;
    OutUseWrapping = UseMessageWrapping;
}

bool UViveTCPSocketObject::IsRunning() const
{
    return Running;
}

void UViveTCPSocketObject::SetRunning( bool InRunning )
{
    Running = InRunning;
}

FSocket* UViveTCPSocketObject::GetSocket() const
{
    return Socket;
}

void UViveTCPSocketObject::SetSocket( FSocket* InSocket )
{
    Socket = InSocket;
}

ISocketSubsystem* UViveTCPSocketObject::GetSocketSubsystem() const
{
    return SocketSubsystem;
}

void UViveTCPSocketObject::SetSocketSubsystem( ISocketSubsystem* InSocketSubsystem )
{
    SocketSubsystem = InSocketSubsystem;
}

FString UViveTCPSocketObject::GetConnectionID() const
{
    return ConnectionID;
}

FString UViveTCPSocketObject::GetIP() const
{
    return IP;
}

int32 UViveTCPSocketObject::GetPort() const
{
    return Port;
}

int32 UViveTCPSocketObject::GetWaitTimeMS() const
{
    return WaitTimeMS;
}

EViveTCPReceiveFilterType UViveTCPSocketObject::GetReceiveFilter() const
{
    return ReceiveFilter;
}

ESocketConnectionState UViveTCPSocketObject::GetConnectionState() const
{
    return Socket->GetConnectionState();
}
