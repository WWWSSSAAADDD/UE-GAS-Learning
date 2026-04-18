// Copyright 


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"


void FGameplayTags::InitializeNativeGameplayTags()
{
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"), FString("减少伤害，提高格挡率"));
}
