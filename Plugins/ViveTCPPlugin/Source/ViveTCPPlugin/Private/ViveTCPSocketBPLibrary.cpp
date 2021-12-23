#include "ViveTCPSocketBPLibrary.h"
#include "ViveTCPSocketBPAPIImpl.h"
#include "ViveTCPSocketServerBPAPIImpl.h"

UViveTCPSocketBPLibrary::UViveTCPSocketBPLibrary( const FObjectInitializer& ObjectInitializer )
    : Super( ObjectInitializer )
{
}

void UViveTCPSocketBPLibrary::GetSocketAPI( TScriptInterface<IViveTCPSocketBPAPI>& OutAPI )
{
    static UViveTCPSocketBPAPIImpl* objInstance = NewObject<UViveTCPSocketBPAPIImpl>( GetTransientPackage(), NAME_None, RF_MarkAsRootSet );
    OutAPI = objInstance;
}

void UViveTCPSocketBPLibrary::GetSocketServerAPI( TScriptInterface<IViveTCPSocketServerBPAPI>& OutAPI )
{
    static UViveTCPSocketServerBPAPIImpl* objInstance = NewObject<UViveTCPSocketServerBPAPIImpl>( GetTransientPackage(), NAME_None, RF_MarkAsRootSet );
    OutAPI = objInstance;
}
