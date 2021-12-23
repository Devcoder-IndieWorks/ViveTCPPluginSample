#include "ViveTCPSessionReceiver.h"
#include "ViveTCPSocketServer.h"
#include "ViveSessionEvent.h"
#include "ViveLog.h"

FViveTCPSessionReceiver::FViveTCPSessionReceiver( UViveTCPSocketServer* InSocketServer, FViveTCPSession* InSession )
    : Stopping( false )
{
    SocketServer = InSocketServer;
    Session = InSession;

    auto threadName = FString::Printf( TEXT( "ViveTCPServerReceiverThread_%s" ), *(FGuid::NewGuid().ToString()) );
    Thread = MakeShareable( FRunnableThread::Create( this, *threadName, 128 * 1024, TPri_Normal ) );
}

FViveTCPSessionReceiver::~FViveTCPSessionReceiver()
{
    Stop();

    if ( Thread.IsValid() ) {
        Thread->Kill( true );
        Thread = nullptr;
    }
}

void FViveTCPSessionReceiver::Stop()
{
    Stopping = true;

    if ( Thread.IsValid() )
        Thread->WaitForCompletion();
}

uint32 FViveTCPSessionReceiver::Run()
{
    if ( !SocketServer->IsRunning() ) {
        VIVELOG( Error, TEXT( "#### TCP socket is not running. ####" ) );
        return 0;
    }

    //-------------------------------------------------------------------------

    FString messageHeader, messageFooter;
    bool messageWrapping;
    SocketServer->GetMessageWrapping( messageHeader, messageFooter, messageWrapping );

    auto messageFooterLineBreak  = FString::Printf( TEXT( "%s\r\n" ), *messageFooter );
    auto messageFooterLineBreak2 = FString::Printf( TEXT( "%s\r" ), *messageFooter );

    //-------------------------------------------------------------------------

    const FTimespan socketTimeout( FTimespan::FromSeconds( 1.0 ) );
    FArrayReaderPtr datagram = MakeShared<FArrayReader, ESPMode::ThreadSafe>( true );

    //-------------------------------------------------------------------------

    auto serverID = SocketServer->GetServerID();
    auto socket = Session->Socket;
    auto sessionID = Session->SessionID;

    //-------------------------------------------------------------------------

    FString mainMessage;
    TArray<uint8> byteDatas;
    bool inCollectMessageStatus = false;

    //-------------------------------------------------------------------------

    while ( !Stopping && SocketServer->IsRunning() ) {
        // [참고] FScoket: Problems with GetConnectionState and Send? - https://answers.unrealengine.com/questions/336960/fsocket-problems-with-getconnectionstate-and-send.html
        // Tick 비교를 해서 연결 끊김을 확인. 연결이 해제되면 Wait()가 작동하지 않으므로 preWait 틱과 postWait 틱이 같아짐.
        auto preWait = FDateTime::Now().GetTicks();
        socket->Wait( ESocketWaitConditions::WaitForRead, socketTimeout );
        auto postWait = FDateTime::Now().GetTicks();

        uint32 dataSize = 0;
        bool hasData = socket->HasPendingData( dataSize );
        if ( !hasData && (preWait == postWait) ) {
            VIVELOG( Display, TEXT( "#### TCP server connection broker. End loop. ####" ) );
            break;
        }

        //---------------------------------------------------------------------

        if ( hasData ) {
            datagram->SetNumUninitialized( dataSize );

            int32 receiveDataSize = 0;
            if ( socket->Recv( datagram->GetData(), datagram->Num(), receiveDataSize ) ) {
                auto recvFilter = SocketServer->GetReceiveFilter();
                if ( messageWrapping && (recvFilter == EViveTCPReceiveFilterType::E_SAB || recvFilter == EViveTCPReceiveFilterType::E_S) ) {
                    datagram->Add( 0x00 ); // Null-terminator.
                    auto data = (char*)datagram->GetData();

                    auto recvMessage = FString( UTF8_TO_TCHAR( data ) );
                    if ( recvMessage.StartsWith( messageHeader ) ) {
                        inCollectMessageStatus = true;
                        recvMessage.RemoveFromStart( messageHeader );
                    }

                    if ( recvMessage.EndsWith( messageFooter ) ) {
                        inCollectMessageStatus = false;
                        recvMessage.RemoveFromEnd( messageFooter );
                    }
                    else if ( recvMessage.EndsWith( messageFooterLineBreak ) ) {
                        inCollectMessageStatus = false;
                        recvMessage.RemoveFromEnd( messageFooterLineBreak );
                        recvMessage.Append( TEXT( "\r\n" ) );
                    }
                    else if ( recvMessage.EndsWith( messageFooterLineBreak2 ) ) {
                        inCollectMessageStatus = false;
                        recvMessage.RemoveFromEnd( messageFooterLineBreak2 );
                        recvMessage.Append( TEXT( "\r" ) );
                    }

                    if ( !inCollectMessageStatus ) {
                        if ( recvMessage.Contains( messageHeader ) ) {
                            TArray<FString> lines;
                            int32 lineCount = recvMessage.ParseIntoArray( lines, *messageHeader );
                            for ( int32 i = 0; i < lineCount; i++ ) {
                                mainMessage = lines[ i ];
                                if ( mainMessage.EndsWith( messageFooter ) ) {
                                    mainMessage.RemoveFromEnd( messageFooter );
                                }
                                else if ( mainMessage.EndsWith( messageFooterLineBreak ) ) {
                                    mainMessage.RemoveFromEnd( messageFooterLineBreak );
                                    mainMessage.Append( TEXT( "\r\n" ) );
                                }
                                else if ( mainMessage.EndsWith( messageFooterLineBreak2 ) ) {
                                    mainMessage.RemoveFromEnd( messageFooterLineBreak2 );
                                    mainMessage.Append( TEXT( "\r" ) );
                                }

                                TWeakObjectPtr<UViveTCPSocketServer> server = SocketServer;
                                // Use game thread.
                                AsyncTask( ENamedThreads::GameThread, [mainMessage, sessionID, byteDatas, serverID, server]{
                                    if ( server.IsValid() ) {
                                        server->ReceiveCallback( sessionID, mainMessage, byteDatas, serverID );
                                        auto sessionEvent = server->GetRegisteredSessionEvent( sessionID );
                                        if ( sessionEvent != nullptr )
                                            sessionEvent->EventCallback( mainMessage, byteDatas );
                                    }
                                } );

                                mainMessage.Empty();
                            }

                            continue;
                        }

                        mainMessage.Append( recvMessage );
                    }
                    else {
                        mainMessage.Append( recvMessage );
                        continue;
                    }

                    if ( mainMessage.IsEmpty() )
                        continue;
                }
                else if ( !messageWrapping ) {
                    if ( recvFilter == EViveTCPReceiveFilterType::E_SAB || recvFilter == EViveTCPReceiveFilterType::E_S ) {
                        datagram->Add( 0x00 );
                        auto data = (char*)datagram->GetData();
                        mainMessage = FString( UTF8_TO_TCHAR( data ) );
                    }

                    if ( recvFilter == EViveTCPReceiveFilterType::E_SAB || recvFilter == EViveTCPReceiveFilterType::E_B )
                        byteDatas.Append( datagram->GetData(), datagram->Num() );
                }

                TWeakObjectPtr<UViveTCPSocketServer> server = SocketServer;
                // Use game thread.
                AsyncTask( ENamedThreads::GameThread, [mainMessage, sessionID, byteDatas, serverID, server]{
                    if ( server.IsValid() ) {
                        server->ReceiveCallback( sessionID, mainMessage, byteDatas, serverID );
                        auto sessionEvent = server->GetRegisteredSessionEvent( sessionID );
                        if ( sessionEvent != nullptr )
                            sessionEvent->EventCallback( mainMessage, byteDatas );
                    }
                } );
            }

            datagram->Empty();
            byteDatas.Empty();
            mainMessage.Empty();
        }
    }

    TWeakObjectPtr<UViveTCPSocketServer> server = SocketServer;
    AsyncTask( ENamedThreads::GameThread, [sessionID, server] {
        if ( server.IsValid() )
            server->RemoveSession( sessionID );
    } );

    if ( socket != nullptr ) {
        socket->Close();

        auto socketSubsystem = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
        if ( socketSubsystem != nullptr ) {
            socketSubsystem->DestroySocket( socket );
            socket = nullptr;
        }
    }

    return 0;
}

