// Copyright 


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Interaction/CombatInterface.h"
#include "Actor/AuraProjectile.h"
#include "AbilitySystemComponent.h"

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
		
		// TODO:给Projectile添加GE，将GE施加给Overlap的Pawn
		
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		Projectile->DamageHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());

		Projectile->FinishSpawning(Transform);
	}
}
