#include "ViveTCPServerThread.h"
#include "ViveTCPSocketServer.h"
#include "ViveLog.h"

FViveTCPServerThread::FViveTCPServerThread( UViveTCPSocketServer* InSocketServer )
    : Stopping( false )
{
    SocketServer = InSocketServer;

    auto threadName = FString::Printf( TEXT( "ViveTCPServerThread_%s" ), *(FGuid::NewGuid().ToString()) );
    Thread = MakeShareable( FRunnableThread::Create( this, *threadName, 8 * 1024, TPri_Normal ) );
}

FViveTCPServerThread::~FViveTCPServerThread()
{
    Stop();

    if ( Thread.IsValid() ) {
        Thread->Kill( true );
        Thread = nullptr;
    }
}

void FViveTCPServerThread::Stop()
{
    Stopping = true;

    if ( Thread.IsValid() )
        Thread->WaitForCompletion();
}

static void ViveTCPServerThread_CloseListenerSocket( FSocket*& InOutSocket, ISocketSubsystem* InSocketSubsystem, 
    const TArray<int32>& InIpArray, int32 InPort, bool InSleep, bool InErrorLog, int32 InErrorCode )
{
    InOutSocket->Close();

    if ( InSleep )
        FPlatformProcess::Sleep( 1 );

    InSocketSubsystem->DestroySocket( InOutSocket );
    InOutSocket = nullptr;
    CVIVELOG( InErrorLog, Error, TEXT( "#### (%i) TCP Server not started. Can't bind %i.%i.%i.%i:%i. Please check IP, Port or your firewall. ####" ), 
        InErrorCode, InIpArray[0], InIpArray[1], InIpArray[2], InIpArray[3], InPort );
}

uint32 FViveTCPServerThread::Run()
{
    TArray<int32> serverIpArray;
    SocketServer->GetIpArray( serverIpArray );

    int32 port = SocketServer->GetPort();
    FString serverID = SocketServer->GetServerID();

    auto socketSubsystem = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
    bool createServer = false;
    FSocket* listenerSocket = nullptr;
    if ( ensure( socketSubsystem != nullptr ) ) {
        auto endPoint = FIPv4Endpoint( FIPv4Address( serverIpArray[0], serverIpArray[1], serverIpArray[2], serverIpArray[3] ), port );
        auto internetAddr = endPoint.ToInternetAddrIPV4();
        listenerSocket = socketSubsystem->CreateSocket( NAME_Stream, TEXT( "SocketServer" ), internetAddr->GetProtocolType() );

        if ( ensure( listenerSocket != nullptr ) ) {
            listenerSocket->SetLinger( false, 0 );

            createServer = listenerSocket->Bind( *internetAddr );
            if ( !createServer ) {
                ViveTCPServerThread_CloseListenerSocket( listenerSocket, socketSubsystem, serverIpArray, port, false, true, 211 );
            }
            else if ( !listenerSocket->Listen( 8 ) ) {
                ViveTCPServerThread_CloseListenerSocket( listenerSocket, socketSubsystem, serverIpArray, port, false, true, 212 );
                createServer = false;
            }
        }
    }

    if ( createServer && (listenerSocket != nullptr) ) {
        SocketServer->SetRunning( true );
        TWeakObjectPtr<UViveTCPSocketServer> server = SocketServer;
        AsyncTask( ENamedThreads::GameThread, [serverID, server]{
            server->ConnectionCallback( EViveSocketServerConnectionType::E_Server, true, TEXT( "TCP Server started." ), TEXT( "" ), serverID );
        } );

        while ( !Stopping ) {
            bool pending;
            listenerSocket->WaitForPendingConnection( pending, FTimespan::FromSeconds( 1 ) );
            if ( pending ) {
                FViveTCPSession session;
                session.SessionID = FGuid::NewGuid().ToString();
                session.ServerID = serverID;

                auto clientAddr = socketSubsystem->CreateInternetAddr();
                auto clientSocket = listenerSocket->Accept( *clientAddr, session.SessionID );

                session.IP = clientAddr.Get().ToString( false );
                session.Port = clientAddr.Get().GetPort();
                session.Socket = clientSocket;
                SocketServer->StartSession( session, SocketServer->GetReceiveFilter() );
            }
        }
    }

    if ( listenerSocket != nullptr )
        ViveTCPServerThread_CloseListenerSocket( listenerSocket, socketSubsystem, serverIpArray, port, true, false, 0 );

    TWeakObjectPtr<UViveTCPSocketServer> server = SocketServer;
    AsyncTask( ENamedThreads::GameThread, [serverID, server]{
        server->ConnectionCallback( EViveSocketServerConnectionType::E_Server, false, TEXT( "TCP Server stopped." ), TEXT( "" ), serverID );
    } );

    return 0;
}
