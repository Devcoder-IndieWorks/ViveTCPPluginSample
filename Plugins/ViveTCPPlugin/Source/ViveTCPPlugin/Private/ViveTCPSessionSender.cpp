#include "ViveTCPSessionSender.h"
#include "ViveTCPSocketServer.h"
#include "ViveLog.h"

FViveTCPSessionSender::FViveTCPSessionSender( UViveTCPSocketServer* InSocketServer, FViveTCPSession* InSession )
    : Stopping( false )
    , Paused( false )
{
    SocketServer = InSocketServer;
    Session = InSession;

    auto threadName = FString::Printf( TEXT( "ViveTCPServerSenderThread_%s" ), *(FGuid::NewGuid().ToString()) );
    Thread = MakeShareable( FRunnableThread::Create( this, *threadName, 128 * 1024, TPri_Normal ) );
}

FViveTCPSessionSender::~FViveTCPSessionSender()
{
    Stop();

    if ( Thread.IsValid() ) {
        Thread->Kill( true );
        Thread = nullptr;
    }
}

void FViveTCPSessionSender::Stop()
{
    Stopping = true;
    PauseThread( false );

    if ( Thread.IsValid() )
        Thread->WaitForCompletion();
}

uint32 FViveTCPSessionSender::Run()
{
    if ( !SocketServer->IsRunning() ) {
        VIVELOG( Error, TEXT( "#### TCP socket server is not running. ####" ) );
        return 0;
    }

    auto socket = Session->Socket;

    while ( !Stopping ) {
        if ( SocketServer->IsRunning() && (!MessageQueue.IsEmpty() || !ByteArrayQueue.IsEmpty()) ) {
            bool isConnected = socket->GetConnectionState() == ESocketConnectionState::SCS_Connected;
            CVIVELOG( !isConnected, Warning, TEXT( "#### TCP socket is not connected. ####" ) );

            if ( isConnected ) {
                int32 bytesSent = 0;
                while ( !MessageQueue.IsEmpty() ) {
                    int32 sent = 0;
                    FString msg;
                    MessageQueue.Dequeue( msg );
                    FTCHARToUTF8 data( *msg );
                    TArray<uint8> byteArray;
                    byteArray.Append( (uint8*)data.Get(), data.Length() );

                    socket->Send( byteArray.GetData(), byteArray.Num(), sent );
                    bytesSent += sent;
                }
                VIVELOG( Log, TEXT( "#### Bytes message sent: [%i] ####" ), bytesSent );

                bytesSent = 0;
                while ( !ByteArrayQueue.IsEmpty() ) {
                    int32 sent = 0;
                    TArray<uint8> byteArray;
                    ByteArrayQueue.Dequeue( byteArray );

                    socket->Send( byteArray.GetData(), byteArray.Num(), sent );
                    bytesSent += sent;
                }
                VIVELOG( Log, TEXT( "#### Bytes array sent: [%i] ####" ), bytesSent );
            }
        }

        if ( SocketServer->IsRunning() ) {
            PauseThread( true );
            while ( Paused && SocketServer->IsRunning() )
                FPlatformProcess::Sleep( 0.01f );
        }
    }

    return 0;
}

void FViveTCPSessionSender::SendMessage( const FString& InMessage, const TArray<uint8>& InByteArray )
{
    if ( InMessage.Len() > 0 )
        MessageQueue.Enqueue( InMessage );

    if ( InByteArray.Num() > 0 )
        ByteArrayQueue.Enqueue( InByteArray );

    PauseThread( false );
}

void FViveTCPSessionSender::PauseThread( bool InPause )
{
    Paused = InPause;
    if ( Thread.IsValid() )
        Thread->Suspend( InPause );
}
