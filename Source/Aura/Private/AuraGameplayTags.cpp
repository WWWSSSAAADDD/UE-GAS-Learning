// Copyright 


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"


void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	/* Vital Attribute */
	Get().Attribute_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Vital.Health"), FString("生命值"));
	Get().Attribute_Vital_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Vital.Mana"), FString("法力值"));

	/* Primary Attribute */
	Get().Attribute_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Strength"), FString("增加物理伤害"));
	Get().Attribute_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Intelligence"), FString("增加法术伤害"));
	Get().Attribute_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Resilience"), FString("增加护甲和护甲穿透"));
	Get().Attribute_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Vigor"), FString("增加生命值"));

	/* Secondary Attribute */
	Get().Attribute_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"), FString("减少伤害，提高格挡率"));
	Get().Attribute_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ArmorPenetration"), FString("增加暴击率、暴击伤害"));
	Get().Attribute_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.BlockChance"), FString("格挡率，降低伤害"));
	Get().Attribute_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitChance"), FString("暴击率"));
	Get().Attribute_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitDamage"), FString("暴击伤害"));
	Get().Attribute_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.CriticalHitResistance"), FString("防暴率"));
	Get().Attribute_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.HealthRegeneration"), FString("每秒恢复生命值"));
	Get().Attribute_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxHealth"), FString("最大生命值"));
	Get().Attribute_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.ManaRegeneration"), FString("每秒恢复法力值"));
	Get().Attribute_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.MaxMana"), FString("最大法力值"));

	/* Resistance Attributes */
	Get().Attribute_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Fire"), FString("火焰抗性"));
	Get().Attribute_Resistance_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Arcane"), FString("奥术抗性"));
	Get().Attribute_Resistance_Lighting = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Lighting"), FString("雷电抗性"));
	Get().Attribute_Resistance_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Resistance.Physical"), FString("物理抗性"));


	/* Input Tags */
	Get().InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.RMB"), FString("鼠标左键"));
	Get().InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.LMB"), FString("鼠标右键"));
	Get().InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.1"), FString("key1"));
	Get().InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.2"), FString("key2"));
	Get().InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.3"), FString("key3"));
	Get().InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.4"), FString("key4"));

	/* DamageTypeTag，用于调用TagSetByCaller */
	Get().Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Fire"), FString("火焰伤害"));
	Get().Damage_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Arcane"), FString("奥术伤害"));
	Get().Damage_Lighting = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Lighting"), FString("雷电伤害"));
	Get().Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Physical"), FString("物理伤害"));

	
	Get().DamageTypesToResistance.Add(Get().Damage_Fire, Get().Attribute_Resistance_Fire);
	Get().DamageTypesToResistance.Add(Get().Damage_Arcane, Get().Attribute_Resistance_Arcane);
	Get().DamageTypesToResistance.Add(Get().Damage_Lighting, Get().Attribute_Resistance_Lighting);
	Get().DamageTypesToResistance.Add(Get().Damage_Physical, Get().Attribute_Resistance_Physical);


	/* Ability Tags，用于激活GA的Tag*/
	/* Effects Tags */
	Get().Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Effects.HitReact"), FString("受击反应"));

	Get().Abilities_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Abilities.Attack"), FString("攻击"));

	/* End Ability Tags*/

	/* Montage Tags，用于Montage发送GameplayEvent的Tag*/
	Get().Montage_Attack_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.Weapon"), FString("此攻击蒙太奇需要Weapon的Socket"));
	Get().Montage_Attack_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.LeftHand"), FString("此攻击蒙太奇需要LeftHand的Socket"));
	Get().Montage_Attack_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Montage.Attack.RightHand"), FString("此攻击蒙太奇需要RightHand的Socket"));
}
