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
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|Attributes")
	static void InitializeDefaultAttributes(const UObject* WorldContext, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);
	
	// 用于给AI敌人初始化Ability，例如GA_HitReact
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|Abilities")
	static void InitializeDefaultAbilities(const UObject* WorldContext, UAbilitySystemComponent* ASC);

	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContext);
};
