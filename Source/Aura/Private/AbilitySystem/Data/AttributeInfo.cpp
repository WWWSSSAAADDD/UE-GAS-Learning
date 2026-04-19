// Copyright 


#include "AbilitySystem/Data/AttributeInfo.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& Tag, bool bLogNotFound) const
{
	for (const FAuraAttributeInfo& AttributeInfo: AttributeInformation)
	{
		if (AttributeInfo.AttributeTag == Tag)
		{
			return AttributeInfo;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AttributeInfo for AttributeTag[%s] on AttributeInfoClass[%s]."), *Tag.ToString(), *GetNameSafe(this));
	}

	return FAuraAttributeInfo();
}
