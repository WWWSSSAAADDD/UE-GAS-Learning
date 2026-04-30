// Copyright 


#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Player/AuraPlayerState.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "Game/AuraGameModeBase.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContext)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
	{
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			FWidgetControllerParams Params(PC, PS, ASC, AS);
			return HUD->GetOverlayWidgetController(Params);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContext)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
	{
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			
			FWidgetControllerParams WCParams(PC, PS, ASC, AS);
			return HUD->GetAttributeMenuWidgetController(WCParams);
		}
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContext, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	UCharacterClassInfo* Info = GetCharacterClassInfo(WorldContext);
	if (!Info) return;
	AActor* Avatar = ASC->GetAvatarActor();

	FGameplayEffectContextHandle PrimaryAttributesContext = ASC->MakeEffectContext();
	PrimaryAttributesContext.AddSourceObject(Avatar);
	FGameplayEffectSpecHandle PrimaryAttributesHandle = ASC->MakeOutgoingSpec(Info->GetClassDefaultInfo(CharacterClass).PrimaryAttributes, Level, PrimaryAttributesContext);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContext = ASC->MakeEffectContext();
	SecondaryAttributesContext.AddSourceObject(Avatar);
	FGameplayEffectSpecHandle SecondaryAttributesHandle = ASC->MakeOutgoingSpec(Info->SecondaryAttributes, Level, SecondaryAttributesContext);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContext = ASC->MakeEffectContext();
	VitalAttributesContext.AddSourceObject(Avatar);
	FGameplayEffectSpecHandle VitalAttributesHandle = ASC->MakeOutgoingSpec(Info->VitalAttributes, Level, VitalAttributesContext);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesHandle.Data.Get());

}

void UAuraAbilitySystemLibrary::InitializeDefaultAbilities(const UObject* WorldContext, UAbilitySystemComponent* ASC)
{
	UCharacterClassInfo* Info = GetCharacterClassInfo(WorldContext);
	check(Info);
	for (TSubclassOf<UGameplayAbility> AbilityClass : Info->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass);
		ASC->GiveAbility(AbilitySpec);
	}
}
UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContext)
{
	AAuraGameModeBase* GameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
	if (GameMode) return GameMode->CharacterClassInfomation;
	return nullptr;
}

bool UAuraAbilitySystemLibrary::IsBlocked(const FGameplayEffectContextHandle& ContextHandle)
{
	if (const FAuraGameplayEffectContext* EffectContext = static_cast<const FAuraGameplayEffectContext*>(ContextHandle.Get()))
	{
		return EffectContext->GetIsBlocked();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsCritical(const FGameplayEffectContextHandle& ContextHandle)
{
	if (const FAuraGameplayEffectContext* EffectContext = static_cast<const FAuraGameplayEffectContext*>(ContextHandle.Get()))
	{
		return EffectContext->GetIsCritical();
	}
	return false;
}

void UAuraAbilitySystemLibrary::SetBlocked(UPARAM(ref)FGameplayEffectContextHandle& ContextHandle, bool bIsBlocked)
{
	if (FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(ContextHandle.Get()))
	{
		EffectContext->SetIsBlocked(bIsBlocked);
	}
	return;
}

void UAuraAbilitySystemLibrary::SetCritical(UPARAM(ref)FGameplayEffectContextHandle& ContextHandle, bool bIsCritical)
{
	if (FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(ContextHandle.Get()))
	{
		EffectContext->SetIsCritical(bIsCritical);
	}
	return;
}
