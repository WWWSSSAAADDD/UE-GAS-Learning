// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetDataUnderMouseSignature, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * 
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwnningAbility", DefaultToSelf = "OwnningAbility", BlueprintInternalUseOnly))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwnningAbility);

	UPROPERTY(BlueprintAssignable)
	FTargetDataUnderMouseSignature OnGetValidData;

protected:
	virtual void Activate() override;


private:
	void SendMouseCursorData();

	void OnMouseCursorDataSent(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag Tag);
};
