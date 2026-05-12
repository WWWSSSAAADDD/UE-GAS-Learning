// Copyright 


#include "Character/AuraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystemComponent.h"
#include "UI/HUD/AuraHUD.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

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
	// 初始化Overlay组件
	InitAbilityActorInfo();

	GrantAbilities();
}

void AAuraCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	// Init Ability Actor Info for the Client 
	// 初始化Overlay组件
	InitAbilityActorInfo();

}

int32 AAuraCharacter::GetPlayerLevel()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

void AAuraCharacter::InitAbilityActorInfo() {
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState)
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// 多人联机时，非本机玩家的GetController返回nullptr，这种情况我们不想让程序崩溃
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 服务器GetHUD返回nullptr，此时不执行InitOverlay
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AuraHUD->InitOverlay(PC, GetPlayerState(), AbilitySystemComponent, AttributeSet);
		}
	}

	if (!HasAuthority()) return;
	/* 初始化Primary、Secondary Attributes */
	InitDefaultAttributes();
}
