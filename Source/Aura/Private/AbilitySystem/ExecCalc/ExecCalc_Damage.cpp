// Copyright


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Interaction/CombatInterface.h"
#include "AuraAbilityTypes.h"

struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightingResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToDefinitions;

	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightingResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);

		// Ensure native gameplay tags are registered before we read them from the FAuraGameplayTags singleton
		FAuraGameplayTags::InitializeNativeGameplayTags();

		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Secondary_Armor, ArmorDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Secondary_BlockChance, BlockChanceDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Secondary_CriticalHitResistance, CriticalHitResistanceDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Secondary_CriticalHitDamage, CriticalHitDamageDef);


		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Resistance_Fire, FireResistanceDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Resistance_Arcane, ArcaneResistanceDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Resistance_Lighting, LightingResistanceDef);
		TagsToDefinitions.Add(FAuraGameplayTags::Get().Attribute_Resistance_Physical, PhysicalResistanceDef);
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
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightingResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	FGameplayEffectSpec EffectSpec = ExecutionParams.GetOwningSpec();
	FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(EffectSpec.GetContext().Get());

	FAggregatorEvaluateParameters Params = FAggregatorEvaluateParameters();
	Params.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Params.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	ICombatInterface* SourceInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetInterface = Cast<ICombatInterface>(TargetAvatar);

	// 捕获SetByCaller的Damge标签的值
	float DamageFromSetByCaller = 0.f;
	for (const TPair<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistance)
	{
		FGameplayTag DamageTypeTag = Pair.Key;
		FGameplayTag DamageResistanceTag = Pair.Value;
		
		float Damage = EffectSpec.GetSetByCallerMagnitude(DamageTypeTag);
		FGameplayEffectAttributeCaptureDefinition ResistanceDef = DamageStatics().TagsToDefinitions[DamageResistanceTag];
		float DamageResistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ResistanceDef, Params, DamageResistance);
		DamageResistance = FMath::Clamp<float>(DamageResistance, 0.f, 100.f);

		DamageFromSetByCaller += Damage * (100 - DamageResistance) / 100;

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

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, Params, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(0.f, SourceCriticalHitDamage);

	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, Params, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);
	
	// 格挡
	if (TargetBlockChance > FMath::FRandRange(0.f, 1.f))
	{
		DamageFromSetByCaller /= 2;
		EffectContext->SetIsBlocked(true);
	}

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
		DamageFromSetByCaller = DamageFromSetByCaller * 2 + SourceCriticalHitDamage;
		EffectContext->SetIsCritical(true);
	}

	// ExecCalc通过添加Modifier，来给Target施加InComingDamage
	FGameplayModifierEvaluatedData InComingDamageMod = FGameplayModifierEvaluatedData(UAuraAttributeSet::GetInComingDamageAttribute(), EGameplayModOp::Additive, DamageFromSetByCaller);
	OutExecutionOutput.AddOutputModifier(InComingDamageMod);
}