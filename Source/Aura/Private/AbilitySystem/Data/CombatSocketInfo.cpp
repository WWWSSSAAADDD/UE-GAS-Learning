// Copyright 


#include "AbilitySystem/Data/CombatSocketInfo.h"
#include "UObject/UnrealNames.h"

FName UCombatSocketInfo::GetCombatSocketName(FGameplayTag MontageTag, bool& bUseWeapon)
{
    for (const FCombatSocketMapping& SocketMapping : CombatSocketConfigs)
    {
        if (SocketMapping.MontageTag.MatchesTagExact(MontageTag))
        {
            bUseWeapon = SocketMapping.bUseWeapon;
            return SocketMapping.CombatSocketName;
        }
    }
    return NAME_None;
}