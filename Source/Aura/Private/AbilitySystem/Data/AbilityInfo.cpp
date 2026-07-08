// Copyright 


#include "AbilitySystem/Data/AbilityInfo.h"

FAuraAbilityInfo UAbilityInfo::FindAbilityInfoForTag(FGameplayTag Tag)
{
	for (const FAuraAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag == Tag)
		{
			return Info;
		}
	}
	return FAuraAbilityInfo();
}