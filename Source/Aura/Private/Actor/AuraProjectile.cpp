// Copyright

#include "Actor/AuraProjectile.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Aura/Aura.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"

// Sets default values
AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 100.f;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere Collision");
	SetRootComponent(Sphere);

	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_CursorTrace, ECR_Ignore);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement Component");
	ProjectileMovementComponent->InitialSpeed = 550.f;
	ProjectileMovementComponent->MaxSpeed = 550.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnProjectileOverlap);

	TrailSoundComponent = UGameplayStatics::SpawnSoundAttached(
	    TrailSound.Get(),
	    GetRootComponent(),
	    NAME_None,
	    FVector::ZeroVector,
	    EAttachLocation::KeepRelativeOffset);
}

void AAuraProjectile::Destroyed()
{
	Super::Destroyed();
}

void AAuraProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!DamageHandle.Data.IsValid() || 
		!UAuraAbilitySystemLibrary::IsNotFriend(DamageHandle.Data.Get()->GetEffectContext().GetEffectCauser(), OtherActor))
	{
		return;
	}

	if (bHit) 
	{
		return;
	}

	AActor* EffectCauser = DamageHandle.Data.Get()->GetEffectContext().GetEffectCauser();
	if (!bHit && EffectCauser != OtherActor)
	{
		bHit = true;
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMovementComponent->StopMovementImmediately();
		MulticastHandleImpactEffect();
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageHandle.Data.Get());
		}
		Destroy();
	}
}

void AAuraProjectile::MulticastHandleImpactEffect_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (TrailSoundComponent)
	{
		TrailSoundComponent->Stop();
	}
}