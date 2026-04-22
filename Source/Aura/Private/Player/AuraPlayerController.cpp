// Copyright 


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"

AAuraPlayerController::AAuraPlayerController() {
    bReplicates = true;
}

void AAuraPlayerController::BeginPlay() {
    Super::BeginPlay();
    check(AuraContext);

    // 
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (Subsystem)
    {
        Subsystem->AddMappingContext(AuraContext, 0);
    }
    
    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false); // We want the cursor to be visible even when the mouse is captured by the viewport, so we set this to false.
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // We don't want the mouse to be locked to the viewport, so we set this to DoNotLock.  
    SetInputMode(InputMode);
}

void AAuraPlayerController::SetupInputComponent() {
    Super::SetupInputComponent();

    UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

    AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::OnAbilityInputPressed, &ThisClass::OnAbilityInputReleased, &ThisClass::OnAbilityInputHeld);
}

void AAuraPlayerController::PlayerTick(float DeltaTime) {
    Super::PlayerTick(DeltaTime);

    //UE_LOG(LogTemp, Log, TEXT("Here is PlayerTick"));
    CursorTrace();
}

void AAuraPlayerController::Move(const FInputActionValue& Value) {
    const FVector2D InputAxisVector = Value.Get<FVector2D>();
    const FRotator ControllerRotation = GetControlRotation();
    const FRotator ControllerYawRotation(0.f, ControllerRotation.Yaw, 0.f);

    const FVector ForwardDirection = FRotationMatrix(ControllerYawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(ControllerYawRotation).GetUnitAxis(EAxis::Y);
    
    if (APawn* ControlledPawn = GetPawn<APawn>()) {
        ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
        ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
    }
}

void AAuraPlayerController::CursorTrace() {
    FHitResult HitResult;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);

    LastActor = ThisActor;
    ThisActor = HitResult.GetActor();

    /* 将Cursor指向的Enemy进行高亮显示，有5种情况
        A.LastActor == null && ThisActor == null
            do nothing
        B.LastActor == null && ThisActor != null
            highlight ThisActor
        C.LastActor != null && ThisActor == null
            unhighlight LastActor
        D.LastActor != null && ThisActor != null && not equal
            unhighlight LastActor && highlight ThisActor
        E.LastActor != null && ThisActor != null && equal
            do nothing
    */

    if (LastActor == nullptr) {
        if (ThisActor == nullptr) {
            // Case A
            //UE_LOG(LogTemp, Log, TEXT("Here is Case A"));
        }
        else {
            // Case B
            ThisActor->HighlightActor();
        }
    }
    else {
        if (ThisActor == nullptr) {
            // Case C
            LastActor->UnHighlightActor();
        }
        else {
            if (ThisActor == LastActor) {
                // Case E
            }
            else {
                LastActor->UnHighlightActor();
                ThisActor->HighlightActor();
            }
        }
    }
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetAuraASC()
{
    if (AuraAbilitySystemComponent == nullptr)
    {
        AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
    }
    return AuraAbilitySystemComponent;
}

void AAuraPlayerController::OnAbilityInputPressed(FGameplayTag Tag)
{

    bTargeting = ThisActor ? true : false;
    Folllow = 0.f;

    // Do Something To Cast AuraASC
    if (GetAuraASC() == nullptr)
    {
        return;
    }
    GetAuraASC()->OnAbilityInputPressed(Tag);
    
}

void AAuraPlayerController::OnAbilityInputReleased(FGameplayTag Tag)
{
    
    if (GetAuraASC() == nullptr)
    {
        return;
    }
    GetAuraASC()->OnAbilityInputReleased(Tag);
}

void AAuraPlayerController::OnAbilityInputHeld(FGameplayTag Tag)
{
    // Input Tag不是Input.LMB时
    if (!FGameplayTags::Get().InputTag_LMB.MatchesTagExact(Tag))
    {
        if (GetAuraASC())
        {
            GetAuraASC()->OnAbilityInputHeld(Tag);
        }
        return;
    }

    // Input Tag是Input.LMB时
    // 如果指向目标，释放技能；否则进行移动
    if (bTargeting)
    {
        if (GetAuraASC())
        {
            GetAuraASC()->OnAbilityInputHeld(Tag);
        }
        return;
    }
    else
    {
        Folllow += GetWorld()->GetDeltaSeconds();
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            CachedLocation = Hit.ImpactPoint;

            if (APawn* ControlledPawn = GetPawn())
            {
                FVector WorldDirection = (CachedLocation - ControlledPawn->GetActorLocation()).GetSafeNormal();
                ControlledPawn->AddMovementInput(WorldDirection);
            }
        }
        
    }
}
