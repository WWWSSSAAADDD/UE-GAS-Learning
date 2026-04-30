// Copyright 


#include "AbilitySystem/AuraAttributeSet.h"
#include "AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "UObject/CoreNetTypes.h"
#include "Net\UnrealNetwork.h"
#include "AuraGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "Player/AuraPlayerController.h"
#include "AuraAbilityTypes.h"

UAuraAttributeSet::UAuraAttributeSet() {
	InitHealth(100.f);
	// InitMaxHealth(200.f);
	InitMana(100.f);
	// InitMaxMana(200.f);

	/* Primary Attributes */
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Primary_Strength, &GetStrengthAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Primary_Intelligence, &GetIntelligenceAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Primary_Resilience, &GetResilienceAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Primary_Vigor, &GetVigorAttribute);


	/* Secondary Attributes */
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_Armor, &GetArmorAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_ArmorPenetration, &GetArmorPenetrationAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_BlockChance, &GetBlockChanceAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_CriticalHitChance, &GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_CriticalHitDamage, &GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_CriticalHitResistance, &GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_HealthRegeneration, &GetHealthRegenerationAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_ManaRegeneration, &GetManaRegenerationAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_MaxHealth, &GetMaxHealthAttribute);
	TagsToAttributes.Add(FAuraGameplayTags::Get().Attribute_Secondary_MaxMana, &GetMaxManaAttribute);


}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/* Primary Attributes */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	/* Vital Attributes */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	/* Secondary Attributes */
	/* DOREPLIFETIME */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0, GetMaxHealth());
	}
	if (Attribute == GetMaxHealthAttribute())
	{

	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0, GetMaxMana());
	}
	if (Attribute == GetMaxManaAttribute())
	{
		
	}
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
	}

	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceAbilitySystemComponent = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceAbilitySystemComponent) && Props.SourceAbilitySystemComponent->AbilityActorInfo.IsValid() && Props.SourceAbilitySystemComponent->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceAbilitySystemComponent->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceAbilitySystemComponent->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}

}


void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(Props.SourceAvatarActor);
	Props.EffectContextHandle.AddActors(Actors);
	FHitResult Hit = FHitResult();
	Hit.Location = Props.SourceAvatarActor->GetActorLocation();
	Props.EffectContextHandle.AddHitResult(Hit);

	if (Data.EvaluatedData.Attribute == GetInComingDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT("SourAvatar: %s"), *Props.SourceAvatarActor->GetName());
		UE_LOG(LogTemp, Warning, TEXT("TargetAvatar: %s"), *Props.TargetAvatarActor->GetName());
		UE_LOG(LogTemp, Warning, TEXT("Health from Actor Applied GE: %f"), GetHealth());
		UE_LOG(LogTemp, Warning, TEXT("Magnitude: %f"), Data.EvaluatedData.Magnitude);
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	if (Data.EvaluatedData.Attribute == GetInComingDamageAttribute())
	{
		const float DamageValue = GetInComingDamage();
		SetInComingDamage(0.f);
		if (DamageValue > 0.f)
		{
			float NewHealth = GetHealth() - DamageValue;
			SetHealth(FMath::Clamp(NewHealth, 0, GetMaxHealth()));
			const bool bFatal = NewHealth <= 0.f;
		
			if (bFatal)
			{
				if (ICombatInterface* Interface = Cast<ICombatInterface>(Props.TargetAvatarActor))
				{
					Interface->Died();
				}
			}
			else
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
				Props.TargetAbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
			}
		}

		const bool bIsBlocked = UAuraAbilitySystemLibrary::IsBlocked(Props.EffectContextHandle);
		const bool bIsCritical = UAuraAbilitySystemLibrary::IsCritical(Props.EffectContextHandle);
		ShowFloatingText(Props, DamageValue, bIsBlocked, bIsCritical);
	}
}


void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float DamageValue, bool bIsBlocked, bool bIsCritical)
{
	if (Props.TargetAvatarActor != Props.SourceAvatarActor)
		{
			if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceController))
			{
				PC->ShowDamageText(DamageValue, Props.TargetAvatarActor, bIsBlocked, bIsCritical);
			}
		}
}


void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const {
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}
