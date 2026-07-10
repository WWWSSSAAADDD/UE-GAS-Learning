// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 * 
 * Singleton containing Native Gameplay Tags
 */
struct FAuraGameplayTags
{
public:
	static FAuraGameplayTags& Get() { static FAuraGameplayTags singleton; return singleton; }
	static void InitializeNativeGameplayTags();

	FGameplayTag Attribute_Vital_Health;
	FGameplayTag Attribute_Vital_Mana;

	FGameplayTag Attribute_Primary_Strength;
	FGameplayTag Attribute_Primary_Intelligence;
	FGameplayTag Attribute_Primary_Resilience;
	FGameplayTag Attribute_Primary_Vigor;

	FGameplayTag Attribute_Secondary_Armor;
	FGameplayTag Attribute_Secondary_ArmorPenetration;
	FGameplayTag Attribute_Secondary_BlockChance;
	FGameplayTag Attribute_Secondary_CriticalHitChance;
	FGameplayTag Attribute_Secondary_CriticalHitDamage;
	FGameplayTag Attribute_Secondary_CriticalHitResistance;
	FGameplayTag Attribute_Secondary_HealthRegeneration;
	FGameplayTag Attribute_Secondary_MaxHealth;
	FGameplayTag Attribute_Secondary_ManaRegeneration;
	FGameplayTag Attribute_Secondary_MaxMana;

	/* Resistance Tags */
	FGameplayTag Attribute_Resistance_Fire;
	FGameplayTag Attribute_Resistance_Lighting;
	FGameplayTag Attribute_Resistance_Arcane;
	FGameplayTag Attribute_Resistance_Physical;

	/* Input Tags */
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;

	// 伤害类型，用于调用TagSetByCaller
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Lighting;
	FGameplayTag Damage_Physical;

	/* Ability Tags，用于激活GA的Tag*/
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;

	FGameplayTag Abilities_Fire_FireBolt;
	
	/* CoolDown Tags */
	FGameplayTag Cooldown_Fire_FireBolt;
    
	
	/* Effect Tags，用Tag来激活GA */
	FGameplayTag Effects_HitReact;

	/* End Ability Tags */

	/* Montage Tags 用于Montage发送GameplayEvent的Tag*/
	/* 目前Montage Tags的GameplayEvent功能与GetSocketLocation功能耦合*/

	/* Socket Tags 用于映射到对应的Socket Name，参见CombatSocketInfo*/
	FGameplayTag Socket_Attack_Weapon;
	FGameplayTag Socket_Attack_LeftHand;
	FGameplayTag Socket_Attack_RightHand;
	FGameplayTag Socket_Attack_Tail;

	/* Montage TAgs*/
	FGameplayTag Montage_Attack_Weapon;
	FGameplayTag Montage_Attack_LeftHand;
	FGameplayTag Montage_Attack_RightHand;
	FGameplayTag Montage_Attack_Tail;

	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistance;

protected:

private:
};