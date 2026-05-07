// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributeMenuWidgetController;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UAbilitySystemBlueprintLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController")
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController")
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContext);

	// 用于给AI敌人初始化属性，主角Aura的初始化属性放在Character里
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContext, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);
	
	// 用于给AI敌人初始化Ability，例如GA_HitReact
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAbilities(const UObject* WorldContext, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffect")
	static bool IsBlocked(const FGameplayEffectContextHandle& ContextHandle);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffect")
	static bool IsCritical(const FGameplayEffectContextHandle& ContextHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffect")
	static void SetBlocked(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, bool bIsBlocked);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffect")
	static void SetCritical(UPARAM(ref) FGameplayEffectContextHandle& ContextHandle, bool bIsBlocked);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffect")
	static void GetActorsWithinRadius(const UObject* WorldContext, TArray<AActor*>& OutActors,
		const TArray<AActor*>& IgnoreActors, float Radius, const FVector& SphereOrigin);
};
