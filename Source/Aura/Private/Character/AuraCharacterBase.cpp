// Copyright 


#include "Character/AuraCharacterBase.h"

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
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const noexcept {
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

