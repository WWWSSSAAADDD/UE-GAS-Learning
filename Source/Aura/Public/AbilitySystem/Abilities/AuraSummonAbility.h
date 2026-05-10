// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	TSubclassOf<APawn> GetRandomMinionClass();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<APawn>> MinionClasses;

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSummonLocations();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AngleDegree = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinDistance = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SummonNums = 5;
};
