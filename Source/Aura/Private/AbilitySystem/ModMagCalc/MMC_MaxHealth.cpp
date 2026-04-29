// Copyright 


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);

}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Catch tags from Source and Target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvalueationParameters;
	EvalueationParameters.SourceTags = SourceTags;
	EvalueationParameters.TargetTags = TargetTags;

	// 捕获Attribute Value
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvalueationParameters, Vigor);
	Vigor = FMath::Max<float>(0.f, Vigor);

	// 捕获Level
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 Level = CombatInterface->GetPlayerLevel();

	// 最终计算公式
	return 80.f + 2.5 * Vigor + 10 * Level;
}
