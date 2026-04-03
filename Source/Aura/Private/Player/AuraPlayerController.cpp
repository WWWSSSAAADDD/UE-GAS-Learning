// Copyright 


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

AAuraPlayerController::AAuraPlayerController() {
    bReplicates = true;
}

void AAuraPlayerController::BeginPlay() {
    Super::BeginPlay();
    check(AuraContext);

    // 
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    check(Subsystem);
    Subsystem->AddMappingContext(AuraContext, 0);

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
