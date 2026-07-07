// Copyright 


#include "Character/AuraCharacterBase.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Aura/Aura.h"
#include <Net\UnrealNetwork.h>
#include "AuraGameplayTags.h"
#include "AbilitySystem/Data/CombatSocketInfo.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	// FName用于绑定到Mesh上名为FName的插槽Socket上（名字必须一样）
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	// 可以只在攻击动作发生的时候开启碰撞，防止错误碰撞以及额外性能开销
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraCharacterBase, bDead, COND_None, REPNOTIFY_Always);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const noexcept {
	return AbilitySystemComponent;
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(FGameplayTag SocketTag)
{
	check(CombatSocketInfo);
	bool bUseWeapon = false;
	FName SocketName = CombatSocketInfo->GetCombatSocketName(SocketTag, bUseWeapon);

	if (bUseWeapon)
	{
		check(Weapon);
		return Weapon->GetSocketLocation(SocketName);
	}
	else
	{
		return GetMesh()->GetSocketLocation(SocketName);
	}

	return FVector();
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation() const
{
	return HitReactMontage;
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontages_Implementation() const
{
	return AttackMontages;
}

FTaggedMontage AAuraCharacterBase::GetSpecificAttackMontage_Implementation(FGameplayTag MontageTag) const
{
	for (const FTaggedMontage& AttackMontage : AttackMontages)
	{
		if (AttackMontage.MontageTag.MatchesTagExact(MontageTag))
		{
			return AttackMontage;
		}
	}
	return FTaggedMontage();
}

UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation() const
{
	return BloodEffect;
}

void AAuraCharacterBase::Died()
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(FAttachmentTransformRules::KeepWorldTransform, true));
	if (HasAuthority()) bDead = true;

	MulticastHandleDeath();
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());

	// 开启物理模拟
	Weapon->SetSimulatePhysics(true);
	// 开启重力
	Weapon->SetEnableGravity(true);
	// 关闭碰撞检测
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	// 调整对WorldStatic通道的碰撞检测
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 溶解特效
	Dissolve();
}

void AAuraCharacterBase::Dissolve()
{
	if (DissolveMaterialInstance)
	{
		UMaterialInstanceDynamic* DynamicInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicInstance);
		StartDissolveTimeline(DynamicInstance);
	}
	if (WeaponDissolveMaterialInstance)
	{
		UMaterialInstanceDynamic* WeaponDynamicInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		Weapon->SetMaterial(0, WeaponDynamicInstance);
		WeaponStartDissolveTimeline(WeaponDynamicInstance);
	}
}

// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAuraCharacterBase::InitAbilityActorInfo()
{

}

void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const 
{
	check(IsValid(GetAbilitySystemComponent()));
	check(IsValid(GameplayEffectClass));
	FGameplayEffectContextHandle EffectContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::GrantAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	ASC->AddGameplayAbilities(StartupAbilities);
}

void AAuraCharacterBase::InitDefaultAttributes() const
{
	if (!HasAuthority()) return;
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}