// Copyright 


#include "Character/AuraEnemy.h"
#include "Aura/Aura.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

AAuraEnemy::AAuraEnemy() {
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

}

void AAuraEnemy::HighlightActor() {
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CustomDepthRedHighlighted);	// 250对应高亮材质
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CustomDepthRedHighlighted);
}

void AAuraEnemy::UnHighlightActor() {
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::BeginPlay() {
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void AAuraEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}
