// Copyright 


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);

	FGameplayTags::Get().Attribute_Secondary_Armor.ToString();
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange,
		FString::Printf(TEXT("Tag: %s"), *FGameplayTags::Get().Attribute_Secondary_Armor.ToString()));
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Blue, FString("Gameplay Effect Applied"));

	FGameplayTagContainer TagContainer;
	GameplayEffectSpec.GetAllAssetTags(TagContainer);

	GameplayEffectAssetTags.Broadcast(TagContainer);

}
