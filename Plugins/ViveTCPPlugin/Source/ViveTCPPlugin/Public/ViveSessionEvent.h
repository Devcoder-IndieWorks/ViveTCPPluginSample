// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "ViveSessionEvent.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams( FViveSessionEventDelegate, FString, InMessage, const TArray<uint8>&, InByteArray );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FViveSessionEventDelegateGroup, FString, InMessage, const TArray<uint8>&, InByteArray );

UCLASS( Blueprintable, BlueprintType )
class VIVETCPPLUGIN_API UViveSessionEvent : public UObject
{
    GENERATED_UCLASS_BODY()
public:
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void AddOnSessionEventDelegate( const FViveSessionEventDelegate& InDelegate );
    UFUNCTION( BlueprintCallable, Category="ViveTCPSocketServer" )
    void RemoveOnSessionEventDelegate( const FViveSessionEventDelegate& InDelegate );

    void EventCallback( const FString& InMessage, const TArray<uint8>& InByteArray );

private:
    FViveSessionEventDelegateGroup OnSessionEventDelegateGroup;
};
