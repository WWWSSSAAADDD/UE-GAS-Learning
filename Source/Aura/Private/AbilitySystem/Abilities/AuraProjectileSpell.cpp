// Copyright 


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Interaction/CombatInterface.h"
#include "Actor/AuraProjectile.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"



bool UAuraProjectileSpell::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
	{
		if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle); Spec && Spec->IsActive())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s blocked while already active on %s"), *GetNameSafe(this), *GetNameSafe(ActorInfo->AvatarActor.Get()));
			return false;
		}
	}

	return true;
}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, FGameplayTag SocketTag)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	FTransform Transform;
	FVector SpawnLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	Transform.SetLocation(SpawnLocation);

	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		Transform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	FRotator Rotation = (ProjectileTargetLocation - SpawnLocation).Rotation();
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
