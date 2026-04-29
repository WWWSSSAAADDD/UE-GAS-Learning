// Copyright 


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Interaction/CombatInterface.h"

struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(InComingDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, InComingDamage, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Source, false);
	}
};

static AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics Definitions;
	return Definitions;
}


UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().InComingDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	FGameplayEffectSpec EffectSpec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters Params = FAggregatorEvaluateParameters();
	Params.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Params.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	ICombatInterface* SourceInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetInterface = Cast<ICombatInterface>(TargetAvatar);

	// 捕获SetByCaller的Damge标签的值
	float DamageFromSetByCaller = EffectSpec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);

	// Block逻辑
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, Params, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	if (TargetBlockChance > FMath::FRandRange(0.f, 1.f))
	{
		DamageFromSetByCaller /= 2;
	}

	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, Params, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, Params, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);
	
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, Params, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(0.f, SourceCriticalHitChance);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, Params, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(0.f, TargetCriticalHitResistance);

	// 护甲穿透
	FRealCurve* ArmorPenetrationRow = CharacterClassInfo->DamageCoefficients->FindCurve(FName("ArmorPenetrationCoefficient"), FString());
	float ArmorPenetrationCoefficient = ArmorPenetrationRow->Eval(SourceInterface->GetPlayerLevel());
	float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient)/100;

	// 护甲减伤
	FRealCurve* ArmorRow = CharacterClassInfo->DamageCoefficients->FindCurve(FName("ArmorCoefficient"), FString());
	float ArmorCoefficient = ArmorRow->Eval(TargetInterface->GetPlayerLevel());
	DamageFromSetByCaller = DamageFromSetByCaller * (100 - EffectiveArmor * ArmorCoefficient)/100;
	
	// 暴击率
	FRealCurve* CriticalHitResistanceRow = CharacterClassInfo->DamageCoefficients->FindCurve(FName("CriticalHitResistanceCoefficient"), FString());
	float CriticalHitResistanceCoefficient = ArmorRow->Eval(TargetInterface->GetPlayerLevel());
	float EffectiveCriticalHitChance = SourceCriticalHitChance * (1 - TargetCriticalHitResistance * CriticalHitResistanceCoefficient);

	if (EffectiveCriticalHitChance > FMath::FRandRange(0.f, 1.f))
	{
		DamageFromSetByCaller *= 2;
	}

	// ExecCalc通过添加Modifier，来给Target施加InComingDamage
	FGameplayModifierEvaluatedData InComingDamageMod = FGameplayModifierEvaluatedData(DamageStatics().InComingDamageProperty, EGameplayModOp::Additive, DamageFromSetByCaller);
	OutExecutionOutput.AddOutputModifier(InComingDamageMod);
}
