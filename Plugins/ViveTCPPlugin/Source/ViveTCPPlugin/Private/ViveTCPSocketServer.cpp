#include "ViveTCPSocketServer.h"
#include "ViveSessionEvent.h"
#include "ViveTCPServerThread.h"
#include "ViveTCPSessionReceiver.h"
#include "ViveTCPSessionSender.h"
#include "ViveSessionEvent.h"
#include "ViveLog.h"

UViveTCPSocketServer::UViveTCPSocketServer( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
    Running = false;
    Port = 0;
    UseMessageWrapping = false;
    ReceiveFilter = EViveTCPReceiveFilterType::E_SAB;
}

void UViveTCPSocketServer::StartServer( const FViveIpAndPortData& InIpPortData, const FString& InIP, int32 InPort, 
    EViveTCPReceiveFilterType InReceiveFilter, const FString& InServerID )
{
    IpArray = InIpPortData.IpArray;
    IP = InIP;
    Port = InPort;
    ReceiveFilter = InReceiveFilter;
    ServerID = InServerID;

    TCPServerThread = MakeShared<FViveTCPServerThread>( this );
}

void UViveTCPSocketServer::StopServer()
{
    if ( Running ) {
        Running = false;

        TArray<FString> sessionIDs;
        for ( auto& session : ClientSessions )
            sessionIDs.Add( session.Key );

        for ( const auto& id : sessionIDs )
            RemoveSession( id );

        if ( TCPServerThread.IsValid() )
            TCPServerThread->Stop();
    }
}

void UViveTCPSocketServer::StartSession( FViveTCPSession& InSession, EViveTCPReceiveFilterType InReceiveFilter )
{
    InSession.ReceiveThread = MakeShared<FViveTCPSessionReceiver>( this, &InSession );
    InSession.SendThread = MakeShared<FViveTCPSessionSender>( this, &InSession );

    AddSession( InSession );
}

void UViveTCPSocketServer::AddSession( const FViveTCPSession& InSession )
{
    if ( !InSession.SessionID.IsEmpty() && !ClientSessions.Contains( InSession.SessionID ) )
        ClientSessions.Add( InSession.SessionID, InSession );
}

const FViveTCPSession* UViveTCPSocketServer::GetSession( const FString& InKey ) const
{
    auto session = ClientSessions.Find( InKey );
    return session;
}

bool UViveTCPSocketServer::GetSessions( TMap<FString, FViveTCPSession>& OutSessions ) const
{
    for ( auto& session : ClientSessions )
        OutSessions.Add( session.Key, session.Value );

    return OutSessions.Num() > 0;
}

void UViveTCPSocketServer::RemoveSession( const FString& InKey )
{
    auto session = ClientSessions.Find( InKey );
    if ( session != nullptr ) {
        if ( session->SendThread.IsValid() ) {
            session->SendThread->Stop();
            session->SendThread = nullptr;
        }

        if ( session->ReceiveThread.IsValid() ) {
            session->ReceiveThread->Stop();
            session->ReceiveThread = nullptr;
        }

        if ( ClientSessions.Remove( InKey ) )
            UnregisterSessionEvent( InKey );
    }
}

void UViveTCPSocketServer::SetConnectionCallback( TFunction<void( EViveSocketServerConnectionType, bool, FString, FString, FString )>&& InCallback )
{
    OnConnection = MoveTemp( InCallback );
}

void UViveTCPSocketServer::SetReceiveCallback( TFunction<void( FString, FString, const TArray<uint8>&, FString )>&& InCallback )
{
    OnReceive = MoveTemp( InCallback );
}

void UViveTCPSocketServer::ConnectionCallback( EViveSocketServerConnectionType InConnectionType, bool InSuccess, const FString& InMessage, const FString& InSessionID, const FString& InServerID )
{
    if ( OnConnection )
        OnConnection( InConnectionType, InSuccess, InMessage, InSessionID, InServerID );
}

void UViveTCPSocketServer::ReceiveCallback( const FString& InSessionID, const FString& InMessage, const TArray<uint8>& InByteArray, const FString& InServerID )
{
    if ( OnReceive )
        OnReceive( InSessionID, InMessage, InByteArray, InServerID );
}

void UViveTCPSocketServer::ActivateMessageWrapping( const FString& InHeader, const FString& InFooter )
{
    MessageHeader = InHeader;
    MessageFooter = InFooter;
    UseMessageWrapping = true;
}

void UViveTCPSocketServer::DeactivateMessageWrapping()
{
    UseMessageWrapping = false;
}

void UViveTCPSocketServer::GetMessageWrapping( FString& OutHeader, FString& OutFooter, bool& OutUseWrapping )
{
    OutHeader = MessageHeader;
    OutFooter = MessageFooter;
    OutUseWrapping = UseMessageWrapping;
}

void UViveTCPSocketServer::SendAll( const TArray<FString>& InSessionIDs, const FString& InMessage, const TArray<uint8>& InByteArray )
{
    for ( auto& id : InSessionIDs )
        SendTo( id, InMessage, InByteArray );
}

void UViveTCPSocketServer::SendTo( const FString& InSessionID, const FString& InMessage, const TArray<uint8>& InByteArray )
{
    auto session = ClientSessions.Find( InSessionID );
    if ( session != nullptr ) {
        auto valid = session->SendThread.IsValid();
        CVIVELOG( valid, Warning, TEXT( "#### The thread for sending data has not yet been initialized. Data is not sent. ####" ) );
        if ( valid ) {
            auto sender = (FViveTCPSessionSender*)session->SendThread.Get();
            sender->SendMessage( InMessage, InByteArray );
        }
    }
}

void UViveTCPSocketServer::RegisterSessionEvent( const FString& InSessionID, UViveSessionEvent*& OutEvent )
{
    auto sessionEvent = NewObject<UViveSessionEvent>( this, UViveSessionEvent::StaticClass() );
    SessionEvents.Add( InSessionID, sessionEvent );
    OutEvent = sessionEvent;
}

void UViveTCPSocketServer::UnregisterSessionEvent( const FString& InSessionID )
{
    if ( !InSessionID.IsEmpty() )
        SessionEvents.Remove( InSessionID );
}

UViveSessionEvent* UViveTCPSocketServer::GetRegisteredSessionEvent( const FString& InSessionID ) const
{
    if ( InSessionID.IsEmpty() )
        return nullptr;

    auto sessionEvent = SessionEvents.Find( InSessionID );
    return sessionEvent != nullptr ? *sessionEvent : nullptr;
}

bool UViveTCPSocketServer::IsRunning() const
{
    return Running;
}

void UViveTCPSocketServer::SetRunning( bool InRunning )
{
    Running = InRunning;
}

FString UViveTCPSocketServer::GetServerID() const
{
    return ServerID;
}

bool UViveTCPSocketServer::GetIpArray( TArray<int32>& OutIpArray ) const
{
    if ( IpArray.Num() == 0 )
        return false;

    if ( OutIpArray.Num() > 0 )
        OutIpArray.Empty();

    OutIpArray.SetNumZeroed( IpArray.Num() );
    for ( int32 idx = 0; idx < IpArray.Num(); ++idx )
        OutIpArray[ idx ] = IpArray[ idx ];

    return true;
}

FString UViveTCPSocketServer::GetIP() const
{
    return IP;
}

int32 UViveTCPSocketServer::GetPort() const
{
    return Port;
}

EViveTCPReceiveFilterType UViveTCPSocketServer::GetReceiveFilter() const
{
    return ReceiveFilter;
}
