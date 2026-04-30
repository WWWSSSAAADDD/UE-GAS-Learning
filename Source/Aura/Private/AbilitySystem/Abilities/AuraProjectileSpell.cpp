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

	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
	{
		FTransform Transform;
		FVector WeaponTipLocation = CombatInterface->GetWeaponTipLocation();
		Transform.SetLocation(WeaponTipLocation);

		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			Transform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FRotator Rotation = (ProjectileTargetLocation - WeaponTipLocation).Rotation();
		Rotation.Pitch = 0.f;
		Projectile->SetActorRotation(Rotation.Quaternion());
		
		
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.SetAbility(this);
		Projectile->DamageHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

		const float DamageValue = DamageMagnitude.GetValueAtLevel(GetAbilityLevel());

		FGameplayTag DamageTag = FAuraGameplayTags::Get().Damage;
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Projectile->DamageHandle, DamageTag, DamageValue);

		Projectile->FinishSpawning(Transform);
	}
}
