#include "ViveTCPSender.h"
#include "ViveTCPSocketObject.h"
#include "ViveLog.h"

FViveTCPSender::FViveTCPSender( UViveTCPSocketObject* InSocketObject )
    : Stopping( false )
    , Paused( false )
{
    SocketObject = InSocketObject;

    auto threadName = FString::Printf( TEXT( "ViveTCPSenderThread_%s" ), *(FGuid::NewGuid().ToString()) );
    Thread = MakeShareable( FRunnableThread::Create( this, *threadName, 128 * 1024, TPri_Normal ) );
}

FViveTCPSender::~FViveTCPSender()
{
    Stop();

    if ( Thread.IsValid() ) {
        Thread->Kill( true );
        Thread = nullptr;
    }
}

void FViveTCPSender::Stop()
{
    Stopping = true;
    PauseThread( false );

    if ( Thread.IsValid() )
        Thread->WaitForCompletion();
}

uint32 FViveTCPSender::Run()
{
    if ( !SocketObject->IsRunning() ) {
        VIVELOG( Error, TEXT( "#### TCP socket is not running. ####" ) );
        return 0;
    }

    while ( !Stopping ) {
        if ( SocketObject->IsRunning() && (!MessageQueue.IsEmpty() || !ByteArrayQueue.IsEmpty()) ) {
            bool isConnected = SocketObject->GetConnectionState() == ESocketConnectionState::SCS_Connected;
            CVIVELOG( !isConnected, Warning, TEXT( "#### TCP socket is not connected. ####" ) );

            if ( isConnected ) {
                int32 bytesSent = 0;
                while ( !MessageQueue.IsEmpty() ) {
                    FString msg;
                    MessageQueue.Dequeue( msg );
                    FTCHARToUTF8 data( *msg );
                    TArray<uint8> byteArray;
                    byteArray.Append( (uint8*)data.Get(), data.Length() );
                    SendBytes( byteArray, bytesSent );
                }
                VIVELOG( Log, TEXT( "#### Bytes message sent: [%i] ####" ), bytesSent );

                bytesSent = 0;
                while ( !ByteArrayQueue.IsEmpty() ) {
                    TArray<uint8> byteArray;
                    ByteArrayQueue.Dequeue( byteArray );
                    SendBytes( byteArray, bytesSent );
                }
                VIVELOG( Log, TEXT( "#### Bytes array sent: [%i] ####" ), bytesSent );
            }
        }

        if ( SocketObject->IsRunning() ) {
            PauseThread( true );
            while ( Paused && SocketObject->IsRunning() )
                FPlatformProcess::Sleep( 0.01f );
        }
    }

    return 0;
}

void FViveTCPSender::SendBytes( const TArray<uint8>& InByteArray, int32& OutSent )
{
    if ( InByteArray.Num() > 0 )
        OutSent += SocketObject->Send( InByteArray );
}

void FViveTCPSender::SendMessage( const FString& InMessage, const TArray<uint8>& InByteArray )
{
    if ( InMessage.Len() > 0 )
        MessageQueue.Enqueue( InMessage );

    if ( InByteArray.Num() > 0 )
        ByteArrayQueue.Enqueue( InByteArray );

    PauseThread( false );
}

void FViveTCPSender::PauseThread( bool InPause )
{
    Paused = InPause;
    if ( Thread.IsValid() )
        Thread->Suspend( InPause );
}
