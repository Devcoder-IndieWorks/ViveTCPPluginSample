// Copyright ViveStudios. All Rights Reserved.
#pragma once
#include "UObject/ObjectMacros.h"
#include "ViveTCPReceiveFilterType.generated.h"

UENUM( BlueprintType ) 
enum class EViveTCPReceiveFilterType : uint8
{
    E_SAB UMETA(DisplayName="Message And Bytes"),
    E_S   UMETA(DisplayName="Message"),
    E_B   UMETA(DisplayName="Bytes")
};
