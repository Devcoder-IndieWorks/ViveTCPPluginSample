#include "ViveTCPConnection.h"
#include "ViveTCPSocketObject.h"
#include "ViveLog.h"

FViveTCPConnection::FViveTCPConnection( UViveTCPSocketObject* InSocketObject )
{
    SocketObject = InSocketObject;
    Socket = nullptr;
    SocketSubsystem = nullptr;

    auto threadName = FString::Printf( TEXT( "ViveTCPConnectionThread_" ), *(FGuid::NewGuid().ToString()) );
    Thread = MakeShareable( FRunnableThread::Create( this, *threadName, 128 * 1024, TPri_Normal ) );
}

FViveTCPConnection::~FViveTCPConnection()
{
    if ( Thread.IsValid() ) {
        Thread->Kill( true );
        Thread = nullptr;
    }

    if ( Socket != nullptr ) {
        Socket->Close();
        SocketSubsystem->DestroySocket( Socket );
        Socket = nullptr;
        SocketSubsystem = nullptr;
    }
}

uint32 FViveTCPConnection::Run()
{
    if ( Socket == nullptr ) {
        SocketSubsystem = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );

        if ( ensure( SocketSubsystem != nullptr ) ) {
            auto internetAddr = SocketSubsystem->CreateInternetAddr();
            bool isValid = false;
            internetAddr->SetIp( *(SocketObject->GetIP()), isValid );
            internetAddr->SetPort( SocketObject->GetPort() );

            CVIVELOG( !isValid, Error, TEXT( "#### Connection failed. IP not valid: [%s:%i] ####" ), *(SocketObject->GetIP()), SocketObject->GetPort() );
            if ( isValid ) {
                Socket = SocketSubsystem->CreateSocket( NAME_Stream, TEXT( "SocketClient" ), internetAddr->GetProtocolType() );

                if ( Socket != nullptr ) {
                    bool connected = Socket->Connect( *internetAddr );
                    CVIVELOG( !connected, Error, TEXT( "#### Connection failed: [%s] ####" ), SocketSubsystem->GetSocketError( SE_GET_LAST_ERROR_CODE ) );
                    if ( connected ) {
                        SocketObject->SetSocket( Socket );
                        SocketObject->SetSocketSubsystem( SocketSubsystem );
                        SocketObject->SetRunning( true );
                        SocketObject->StartReceiver();
                        SocketObject->StartSender();

                        TWeakObjectPtr<UViveTCPSocketObject> socket = SocketObject;
                        AsyncTask( ENamedThreads::GameThread, [socket]{
                            if ( socket.IsValid() ) {
                                socket->ConnectionCallback( true, FString::Printf( TEXT( "Init TCP connection OK. IP: [%s]" ), *(socket->GetIP()) ), 
                                    socket->GetConnectionID() );
                                socket->Send( FString::Printf( TEXT( "Connected to TCP server. IP:[%s]" ), *(socket->GetIP()) ), TArray<uint8>() );
                            }
                        } );
                    }
                }
            }
        }
    }

    return 0;
}
