// Copyright 


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

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

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::PlayerTick(float DeltaTime) {
    Super::PlayerTick(DeltaTime);

    UE_LOG(LogTemp, Log, TEXT("Here is PlayerTick"));
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
            UE_LOG(LogTemp, Log, TEXT("Here is Case A"));
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
