// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"



/**

 * 
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	// Ability的初始Tag
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartInputTag;

	// 记录Ability的伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FScalableFloat DamageMagnitude;
};
