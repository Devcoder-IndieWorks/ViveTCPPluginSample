// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveTCPSocketBPLibrary.generated.h"

UCLASS() 
class VIVETCPPLUGIN_API UViveTCPSocketBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_UCLASS_BODY()
public:
    UFUNCTION( BlueprintPure, meta=(DisplayName="Vive TCPSocket Module"), Category="ViveTCPSocket" )
    static void GetSocketAPI( TScriptInterface<class IViveTCPSocketBPAPI>& OutAPI );
    UFUNCTION( BlueprintPure, meta=(DisplayName="Vive TCPSocket Server Module"), Category="ViveTCPSocket" )
    static void GetSocketServerAPI( TScriptInterface<class IViveTCPSocketServerBPAPI>& OutAPI );
};
