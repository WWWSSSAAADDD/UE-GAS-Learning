// Copyright 


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Interaction/CombatInterface.h"
#include "Actor/AuraProjectile.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	FTransform Transform;
	FVector WeaponTipLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), FAuraGameplayTags::Get().Montage_Attack_Weapon);
	Transform.SetLocation(WeaponTipLocation);

	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		Transform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	FRotator Rotation = (ProjectileTargetLocation - WeaponTipLocation).Rotation();
	Projectile->SetActorRotation(Rotation.Quaternion());
		
		
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

	/* 尝试给Context添加信息 */
	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(Projectile);
	ContextHandle.AddActors(Actors);
	FHitResult Hit;
	Hit.Location = ProjectileTargetLocation;
	ContextHandle.AddHitResult(Hit);

	ContextHandle.SetAbility(this);
	Projectile->DamageHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

	for (TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypesToScalalbleFloat)
	{
		FGameplayTag& DamageTypeTag = Pair.Key;
		FScalableFloat& DamageScalableFloat = Pair.Value;
		const float DamageValue = DamageScalableFloat.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Projectile->DamageHandle, DamageTypeTag, DamageValue);
	}

	Projectile->FinishSpawning(Transform);

}
