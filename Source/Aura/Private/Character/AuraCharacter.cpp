// Copyright 


#include "Character/AuraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystemComponent.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter() {
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AAuraCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	// Init Ability Actor Info for the Server
	InitAbilityActorInfo();

	InitAuraHUD();
}

void AAuraCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	// Init Ability Actor Info for the Client
	InitAbilityActorInfo();

	InitAuraHUD();
}

void AAuraCharacter::InitAbilityActorInfo() {
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState)
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
	AttributeSet = AuraPlayerState->GetAttributeSet();
}

void AAuraCharacter::InitAuraHUD()
{
	// 多人联机时，非本机玩家的GetController返回nullptr，这种情况我们不想让程序崩溃
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		Cast<AAuraHUD>(PC->GetHUD())->InitOverlay(PC, GetPlayerState, AbilitySystemComponent, AttributeSet);
	}

}