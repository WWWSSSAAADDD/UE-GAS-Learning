// Copyright 


#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* InActor)
{
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, 1.f, FGameplayEffectContextHandle());
	
	for (auto& Pair : DamageTypesToScalalbleFloat)
	{
		float ScalableValue = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		SpecHandle.Data->SetByCallerTagMagnitudes.Add(Pair.Key, ScalableValue);
	}
	
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}
