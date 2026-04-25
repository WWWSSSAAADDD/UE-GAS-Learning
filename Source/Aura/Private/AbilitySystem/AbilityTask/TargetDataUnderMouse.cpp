// Copyright 


#include "AbilitySystem/AbilityTask/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwnningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwnningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool IsLocalPlayer = Ability->GetActorInfo().IsLocallyControlledPlayer();
	if (IsLocalPlayer)
	{
		SendMouseCursorData();
	}
	else
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey PredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, PredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnMouseCursorDataSent);
		const bool bDelegateCalled = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, PredictionKey);
		if (!bDelegateCalled)
		{
			SetWaitingOnRemotePlayerData();
		}
	}

}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	FScopedPredictionWindow ScopePrediction(AbilitySystemComponent.Get());
	
	FHitResult Hit;
	if (APlayerController* PC = Ability->GetActorInfo().PlayerController.Get())
	{

		PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	}

	FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(HitData);

	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle, 
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnGetValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnMouseCursorDataSent(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag Tag)
{
	AbilitySystemComponent.Get()->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	OnGetValidData.Broadcast(DataHandle);
}
