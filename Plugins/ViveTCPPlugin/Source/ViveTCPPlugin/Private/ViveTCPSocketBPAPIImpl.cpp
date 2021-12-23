#include "ViveTCPSocketBPAPIImpl.h"
#include "ViveTCPSocketObject.h"
#include "ViveLog.h"

UViveTCPSocketBPAPIImpl::UViveTCPSocketBPAPIImpl( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void UViveTCPSocketBPAPIImpl::AddOnTCPConnectionDelegate( const FViveTCPConnectionDelegate& InDelegate )
{
    OnTCPConnectionDelegateGroup.Add( InDelegate );
}

void UViveTCPSocketBPAPIImpl::RemoveOnTCPConnectionDelegate( const FViveTCPConnectionDelegate& InDelegate )
{
    OnTCPConnectionDelegateGroup.Remove( InDelegate );
}

void UViveTCPSocketBPAPIImpl::AddOnTCPDisconnectionDelegate( const FViveTCPDisconnectionDelegate& InDelegate )
{
    OnTCPDisconnectionDelegateGroup.Add( InDelegate );
}

void UViveTCPSocketBPAPIImpl::RemoveOnTCPDisconnectionDelegate( const FViveTCPDisconnectionDelegate& InDelegate )
{
    OnTCPDisconnectionDelegateGroup.Remove( InDelegate );
}

void UViveTCPSocketBPAPIImpl::AddOnTCPReceiveDelegate( const FViveTCPReceiveDelegate& InDelegate )
{
    OnTCPReceiveDelegateGroup.Add( InDelegate );
}

void UViveTCPSocketBPAPIImpl::RemoveOnTCPReceiveDelegate( const FViveTCPReceiveDelegate& InDelegate )
{
    OnTCPReceiveDelegateGroup.Remove( InDelegate );
}

void UViveTCPSocketBPAPIImpl::InitTCPSocket( FString& OutConnectionID, const FString& InIP, int32 InPort, 
    int32 InWaitTimeMS, EViveTCPReceiveFilterType InReceiveFilters )
{
    if ( ensure( OnTCPConnectionDelegateGroup.IsBound() )    && 
         ensure( OnTCPDisconnectionDelegateGroup.IsBound() ) && 
         ensure( OnTCPReceiveDelegateGroup.IsBound() )         ) {

        OutConnectionID = FGuid::NewGuid().ToString();
        
        auto socketObj = NewObject<UViveTCPSocketObject>( this, NAME_None );
        socketObj->SetConnectionCallback( [this]( bool success, const FString& msg, const FString& id ){
            if ( OnTCPConnectionDelegateGroup.IsBound() )
                OnTCPConnectionDelegateGroup.Broadcast( success, msg, id );
        } );
        socketObj->SetDisconnectionCallback( [this]{
            if ( OnTCPDisconnectionDelegateGroup.IsBound() )
                OnTCPDisconnectionDelegateGroup.Broadcast();
        } );
        socketObj->SetReceiveCallback( [this]( const FString& msg, const TArray<uint8>& byteArr, const FString& id ){
            if ( OnTCPReceiveDelegateGroup.IsBound() )
                OnTCPReceiveDelegateGroup.Broadcast( msg, byteArr, id );
        } );

        socketObj->Setup( InIP, InPort, OutConnectionID, InWaitTimeMS, InReceiveFilters );
        socketObj->Connection();

        TCPSocketObjects.Add( OutConnectionID, socketObj );
    }
}

void UViveTCPSocketBPAPIImpl::CloseTCPSocket( const FString& InConnectionID )
{
    auto socketObj = GetSocketObject( InConnectionID );

    if ( socketObj != nullptr ) {
        socketObj->Disconnection();
        TCPSocketObjects.Remove( InConnectionID );
    }
}

void UViveTCPSocketBPAPIImpl::SendTo( const FString& InConnectionID, FString InMessage, TArray<uint8> InByteArray, bool InAddLineBreak )
{
    auto socketObj = GetSocketObject( InConnectionID );

    if ( socketObj != nullptr ) {
        if ( InMessage.Len() > 0 ) {
            FString header, footer;
            bool wrapping;
            socketObj->GetMessageWrapping( header, footer, wrapping );
            if ( wrapping )
                InMessage = header + InMessage + footer;

            if ( InAddLineBreak )
                InMessage.Append( TEXT( "\r\n" ) );
        }

        socketObj->Send( InMessage, InByteArray );
    }
}

bool UViveTCPSocketBPAPIImpl::IsTCPConnected( const FString& InConnectionID )
{
    auto socketObj = GetSocketObject( InConnectionID );
    return (socketObj != nullptr) ? socketObj->IsRunning() : false;
}

void UViveTCPSocketBPAPIImpl::ActivateMessageWrapping( const FString& InConnectionID, FString InHeader, FString InFooter )
{
    auto socketObj = GetSocketObject( InConnectionID );

    if ( socketObj != nullptr )
        socketObj->ActivateMessageWrapping( InHeader, InFooter );
}

void UViveTCPSocketBPAPIImpl::DeactivateMessageWrapping( const FString& InConnectionID )
{
    auto socketObj = GetSocketObject( InConnectionID );

    if ( socketObj != nullptr )
        socketObj->DeactivateMessageWrapping();
}

UViveTCPSocketObject* UViveTCPSocketBPAPIImpl::GetSocketObject( const FString& InConnectionID ) const
{
    CVIVELOG( InConnectionID.IsEmpty(), Warning, TEXT( "#### Connection ID is empty. ####" ) );
    if ( InConnectionID.IsEmpty() )
        return nullptr;

    auto findObj = TCPSocketObjects.Find( InConnectionID );
    CVIVELOG( (findObj == nullptr), Warning, TEXT( "#### Connection not found. [%s] ####" ), *InConnectionID );
    return (findObj != nullptr) ? *findObj : nullptr;
}
