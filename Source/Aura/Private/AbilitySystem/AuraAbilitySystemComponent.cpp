// Copyright 


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);

	FGameplayTags::Get().Attribute_Secondary_Armor.ToString();
}

void UAuraAbilitySystemComponent::AddGameplayAbilities(const TArray<TSubclassOf<UGameplayAbility>>& AbilityClasses)
{
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilityClasses)
	{
		UAuraGameplayAbility* AuraAbilityClass = CastChecked<UAuraGameplayAbility>(AbilityClass);
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AuraAbilityClass, 1);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{

	FGameplayTagContainer TagContainer;
	GameplayEffectSpec.GetAllAssetTags(TagContainer);

	GameplayEffectAssetTags.Broadcast(TagContainer);

}
