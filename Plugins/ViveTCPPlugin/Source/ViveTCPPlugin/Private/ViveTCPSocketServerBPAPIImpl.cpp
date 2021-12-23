#include "ViveTCPSocketServerBPAPIImpl.h"
#include "ViveTCPSocketServer.h"
#include "ViveLog.h"

UViveTCPSocketServerBPAPIImpl::UViveTCPSocketServerBPAPIImpl( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void UViveTCPSocketServerBPAPIImpl::AddOnTCPServerConnectionDelegate( const FViveTCPServerConnectionDelegate& InDelegate )
{
    OnTCPServerConnectionDelegateGroup.Add( InDelegate );
}

void UViveTCPSocketServerBPAPIImpl::RemoveOnTCPServerConnectionDelegate( const FViveTCPServerConnectionDelegate& InDelegate )
{
    OnTCPServerConnectionDelegateGroup.Remove( InDelegate );
}

void UViveTCPSocketServerBPAPIImpl::AddOnTCPServerReceiveDelegate( const FViveTCPServerReceiveDelegate& InDelegate )
{
    OnTCPServerReceiveDelegateGroup.Add( InDelegate );
}

void UViveTCPSocketServerBPAPIImpl::RemoveOnTCPServerReceiveDelegate( const FViveTCPServerReceiveDelegate& InDelegate )
{
    OnTCPServerReceiveDelegateGroup.Remove( InDelegate );
}

void UViveTCPSocketServerBPAPIImpl::StartTCPSocketServer( FString& OutServerID, const FString& InIP, int32 InPort, 
    EViveTCPReceiveFilterType InReceiveFilter, FString InCustomServerID )
{
    OutServerID = InCustomServerID;
    if ( OutServerID.IsEmpty() )
        OutServerID = FGuid::NewGuid().ToString();

    if ( TCPSocketServers.Contains( OutServerID ) ) {
        VIVELOG( Warning, TEXT( "#### An server with this server id is already started. [%s] ####" ), *OutServerID );
        return;
    }

    auto ipPortData = CheckIpAndPort( InIP, InPort );
    if ( !ipPortData.Success ) {
        VIVELOG( Warning, TEXT( "#### Can't start server on ip:[%s] server id:[%s] ####" ), *InIP, *OutServerID );
        return;
    }

    for ( auto& server : TCPSocketServers ) {
        auto socketServer = server.Value;
        if ( socketServer != nullptr ) {
            if ( socketServer->GetIP().Equals( InIP ) && socketServer->GetPort() == InPort ) {
                VIVELOG( Warning, TEXT( "#### Can't start server. Address is already use by: ip:[%s] port:[%i] server id:[%s] ####" ),
                    *InIP, InPort, *OutServerID );
                return;
            }
        }
    }

    auto socketServer = NewObject<UViveTCPSocketServer>( this, NAME_None );
    socketServer->SetConnectionCallback( 
        [this]( EViveSocketServerConnectionType type, bool success, FString msg, FString sessionId, FString serverId ){
            if ( OnTCPServerConnectionDelegateGroup.IsBound() )
                OnTCPServerConnectionDelegateGroup.Broadcast( type, success, msg, sessionId, serverId );
        } );
    socketServer->SetReceiveCallback( 
        [this]( FString sessionId, FString msg, const TArray<uint8>& byteArray, FString serverId ){
            if ( OnTCPServerReceiveDelegateGroup.IsBound() )
                OnTCPServerReceiveDelegateGroup.Broadcast( sessionId, msg, byteArray, serverId );
        } );

    socketServer->StartServer( ipPortData, InIP, InPort, InReceiveFilter, OutServerID );
    TCPSocketServers.Add( OutServerID, socketServer );

    LastTCPSocketServerID = OutServerID;
}

void UViveTCPSocketServerBPAPIImpl::StopAllTCPSocketServer()
{
    TArray<FString> serverIDs;
    for ( auto& server : TCPSocketServers )
        serverIDs.Add( server.Key );

    for ( auto& id : serverIDs )
        StopTCPSocketServer( id );
}

void UViveTCPSocketServerBPAPIImpl::StopTCPSocketServer( const FString& InServerID )
{
    FString id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto socketServer = GetSocketServer( id );
    if ( socketServer == nullptr )
        return;

    socketServer->StopServer();
    TCPSocketServers.Remove( id );
}

void UViveTCPSocketServerBPAPIImpl::GetSocketServerSessionIDs( const FString& InServerID, TArray<FString>& OutSessionIDs )
{
    FString id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto socketServer = GetSocketServer( id );
    if ( socketServer == nullptr )
        return;

    TMap<FString, FViveTCPSession> sessions;
    if ( socketServer->GetSessions( sessions ) )
        sessions.GetKeys( OutSessionIDs );
}

void UViveTCPSocketServerBPAPIImpl::GetSocketServerSessionInfo( const FString& InSessionID, bool& OutSessionFound, FString& OutIP, int32& OutPort, FString& OutServerID )
{
    if ( InSessionID.IsEmpty() ) {
        OutSessionFound = false;
        return;
    }

    for ( auto& server : TCPSocketServers ) {
        GetSocketServerSessionInfoByServerID( server.Key, InSessionID, OutSessionFound, OutIP, OutPort );
        if ( OutSessionFound ) {
            OutServerID = server.Key;
            break;
        }
    }
}

void UViveTCPSocketServerBPAPIImpl::GetSocketServerSessionInfoByServerID( const FString& InServerID, const FString& InSessionID, bool& OutSessionFound, FString& OutIP, int32& OutPort )
{
    OutSessionFound = false;
    OutIP = TEXT( "" );
    OutPort = -1;

    if ( InSessionID.IsEmpty() )
        return;

    auto socketServer = GetSocketServer( InServerID );
    if ( socketServer == nullptr )
        return;

    auto session = socketServer->GetSession( InSessionID );
    if ( session != nullptr ) {
        OutSessionFound = true;
        OutIP = session->IP;
        OutPort = session->Port;
    }
}

void UViveTCPSocketServerBPAPIImpl::RemoveSessionAndCloseConnection( const FString& InServerID, const FString& InSessionID )
{
    CVIVELOG( InSessionID.IsEmpty(), Warning, TEXT( "#### Invalid session id. ####" ) );
    if ( InSessionID.IsEmpty() )
        return;

    auto socketServer = GetSocketServer( InServerID );
    if ( socketServer == nullptr )
        return;

    socketServer->RemoveSession( InSessionID );
}

void UViveTCPSocketServerBPAPIImpl::ActivateMessageWrapping( const FString& InServerID, FString InHeader, FString InFooter )
{
    auto socketServer = GetSocketServer( InServerID );
    if ( socketServer != nullptr )
        socketServer->ActivateMessageWrapping( InHeader, InFooter );
}

void UViveTCPSocketServerBPAPIImpl::DeactivateMessageWrapping( const FString& InServerID )
{
    auto socketServer = GetSocketServer( InServerID );
    if ( socketServer != nullptr )
        socketServer->DeactivateMessageWrapping();
}

void UViveTCPSocketServerBPAPIImpl::GetMessageWrapping( const FString& InServerID, FString& OutHeader, FString& OutFooter, bool& OutUseWrapping )
{
    auto socketServer = GetSocketServer( InServerID );
    if ( socketServer != nullptr )
        socketServer->GetMessageWrapping( OutHeader, OutFooter, OutUseWrapping );
}

void UViveTCPSocketServerBPAPIImpl::SendAll( const FString& InServerID, const TArray<FString>& InSessionIDs, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak )
{
    if ( InMessage.IsEmpty() && (InByteArray.Num() == 0) )
        return;

    auto id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto socketServer = GetSocketServer( id );
    if ( socketServer == nullptr )
        return;

    if ( !InMessage.IsEmpty() ) {
        FString header, footer;
        bool useMessageWrapping = false;
        socketServer->GetMessageWrapping( header, footer, useMessageWrapping );

        if ( useMessageWrapping )
            InMessage = header + InMessage + footer;

        if ( InAddLineBreak )
            InMessage.Append( TEXT( "\r\n" ) );
    }

    socketServer->SendAll( InSessionIDs, InMessage, InByteArray );
}

void UViveTCPSocketServerBPAPIImpl::SendTo( const FString& InServerID, const FString& InSessionID, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak )
{
    if ( InMessage.IsEmpty() && (InByteArray.Num() == 0) )
        return;

    auto id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto socketServer = GetSocketServer( id );
    if ( socketServer == nullptr )
        return;

    if ( !InMessage.IsEmpty() ) {
        FString header, footer;
        bool useMessageWrapping = false;
        socketServer->GetMessageWrapping( header, footer, useMessageWrapping );

        if ( useMessageWrapping )
            InMessage = header + InMessage + footer;

        if ( InAddLineBreak )
            InMessage.Append( TEXT( "\r\n" ) );
    }

    socketServer->SendTo( InSessionID, InMessage, InByteArray );
}

void UViveTCPSocketServerBPAPIImpl::RegisterSessionEvent(  const FString& InServerID, const FString& InSessionID, UViveSessionEvent*& OutEvent )
{
    auto id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto socketServer = GetSocketServer( id );
    if ( socketServer != nullptr )
        socketServer->RegisterSessionEvent( InSessionID, OutEvent );
}

void UViveTCPSocketServerBPAPIImpl::UnregisterSessionEvent(  const FString& InServerID, const FString& InSessionID )
{
    auto id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto socketServer = GetSocketServer( id );
    if ( socketServer != nullptr )
        socketServer->UnregisterSessionEvent( InSessionID );
}

UViveTCPSocketServer* UViveTCPSocketServerBPAPIImpl::GetSocketServer( const FString& InServerID ) const
{
    CVIVELOG( InServerID.IsEmpty(), Warning, TEXT( "#### Invalid server id. ####" ) );
    auto id = !InServerID.IsEmpty() ? InServerID : LastTCPSocketServerID;
    auto findServer = TCPSocketServers.Find( id );
    CVIVELOG( findServer == nullptr, Warning, TEXT( "#### Not found socket server. server id:[%s] ####" ), *id );
    return findServer != nullptr ? *findServer : nullptr;
}

FViveIpAndPortData UViveTCPSocketServerBPAPIImpl::CheckIpAndPort( const FString& InIP, int32 InPort )
{
    FViveIpAndPortData ipAndPortData;
    ipAndPortData.Port = -1;
    ipAndPortData.Success = false;

    TArray<FString> ipStrings;
    auto lineCount = InIP.ParseIntoArray( ipStrings, TEXT( "." ) );
    if ( lineCount != 4 ) {
        VIVELOG( Error, TEXT( "#### Invalid IP: %s ####" ), *InIP );
        ipAndPortData.ErrorMessage = TEXT( "Invalid IP: " ) + InIP;
        return ipAndPortData;
    }

    TArray<int32> ipArray = { FCString::Atoi( *ipStrings[ 0 ] ), FCString::Atoi( *ipStrings[ 1 ] ), FCString::Atoi( *ipStrings[ 2 ] ), FCString::Atoi( *ipStrings[ 3 ] ) };
    if ( (ipArray[ 0 ] < 0 || ipArray[ 0 ] > 255) || 
         (ipArray[ 1 ] < 0 || ipArray[ 1 ] > 255) || 
         (ipArray[ 2 ] < 0 || ipArray[ 2 ] > 255) || 
         (ipArray[ 3 ] < 0 || ipArray[ 3 ] > 255) ) {
        VIVELOG( Error, TEXT( "#### Invalid IP: %s ####" ), *InIP );
        ipAndPortData.ErrorMessage = TEXT( "Invalid IP: " ) + InIP;
        return ipAndPortData;
    }

    if ( InPort < 0 || InPort > 65535 ) {
        VIVELOG( Error, TEXT( "#### Invalid Port: %i ####" ), InPort );
        ipAndPortData.ErrorMessage = TEXT( "Invalid Port: " ) + FString::FromInt( InPort );
        return ipAndPortData;
    }

    ipAndPortData.IpArray = ipArray;
    ipAndPortData.Port = InPort;
    ipAndPortData.Success = true;
    return ipAndPortData;
}
