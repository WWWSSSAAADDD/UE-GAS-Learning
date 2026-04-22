// Copyright 


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindInputActionForTag(const FGameplayTag& Tag, bool bLogNotFound) const
{
	for (const FAuraInputAction& AuraIA : AbilityInputActions)
	{
		if (AuraIA.InputAction && AuraIA.GameplayTag == Tag)
		{
			return AuraIA.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT(
			"Class:[%s]的FindInputActionForTag函数无法通过Tag:[%s]找到对应的InputAction"),
			*GetNameSafe(this), *Tag.ToString());
	}

	return nullptr;
}
