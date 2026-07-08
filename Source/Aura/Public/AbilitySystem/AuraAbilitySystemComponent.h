// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

class UAuraAbilitySystemComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FGameplayEffectAssetTagsSignature, const FGameplayTagContainer&);
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitiesGivenDelegateSignature, UAuraAbilitySystemComponent*);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	// 当ASC的ActorInfo设置好以后，被调用。目前用于绑定ASC的回调函数
	void AbilityActorInfoSet();

	//  广播GE的AssetTags，
	FGameplayEffectAssetTagsSignature GameplayEffectAssetTagsDelegate;
	bool bStartupAbilitiesGiven = false;
	FAbilitiesGivenDelegateSignature AbilitiesGivenDelegate;
	void ForEachAbility(const FForEachAbility& Delegate);
	
	void AddGameplayAbilities(const TArray<TSubclassOf<UGameplayAbility>>& AbilityClasses);

	void OnAbilityInputPressed(FGameplayTag InputTag);
	void OnAbilityInputReleased(FGameplayTag InputTag);
	void OnAbilityInputHeld(FGameplayTag InputTag);

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
protected:

	// 当GE对自己生效以后，会触发此函数。目前用于广播GE AssetTags，让OverlayWidgetController判断MessageTag来生成MessageUI
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);

	virtual void OnRep_ActivateAbilities() override;
private:
};
