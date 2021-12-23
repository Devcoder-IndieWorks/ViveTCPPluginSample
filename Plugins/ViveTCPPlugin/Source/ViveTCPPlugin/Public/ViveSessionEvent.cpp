#include "ViveSessionEvent.h"

UViveSessionEvent::UViveSessionEvent( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void UViveSessionEvent::AddOnSessionEventDelegate( const FViveSessionEventDelegate& InDelegate )
{
    OnSessionEventDelegateGroup.Add( InDelegate );
}

void UViveSessionEvent::RemoveOnSessionEventDelegate( const FViveSessionEventDelegate& InDelegate )
{
    OnSessionEventDelegateGroup.Remove( InDelegate );
}

void UViveSessionEvent::EventCallback( const FString& InMessage, const TArray<uint8>& InByteArray )
{
    if ( OnSessionEventDelegateGroup.IsBound() )
        OnSessionEventDelegateGroup.Broadcast( InMessage, InByteArray );
}
