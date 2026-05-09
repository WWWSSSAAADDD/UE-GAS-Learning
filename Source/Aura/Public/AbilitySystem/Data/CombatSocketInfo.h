// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CombatSocketInfo.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct FCombatSocketMapping
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag SocketTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName CombatSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	bool bUseWeapon = false;
};

UCLASS()
class AURA_API UCombatSocketInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FCombatSocketMapping> CombatSocketConfigs;

	FName GetCombatSocketName(FGameplayTag MontageTag, bool& bUseWeapon);
};
