// Copyright 


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Components/SplineComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "UI/Widget/DamageTextComponent.h"

AAuraPlayerController::AAuraPlayerController() {
    bReplicates = true;

    NavSpline = CreateDefaultSubobject<USplineComponent>("Navigation Spline");
}

void AAuraPlayerController::BeginPlay() {
    Super::BeginPlay();
    check(AuraContext);

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
    AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnShiftPressed);
    AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::OnShiftReleased);
    AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::OnAbilityInputPressed, &ThisClass::OnAbilityInputReleased, &ThisClass::OnAbilityInputHeld);
}

void AAuraPlayerController::PlayerTick(float DeltaTime) {
    Super::PlayerTick(DeltaTime);

    CursorTrace();
    AutoRun();
}

void AAuraPlayerController::AutoRun()
{
    if (bAutoRunning)
    {
        if (APawn* ControlledPawn = GetPawn())
        {
            // ActorLocation得到的NavSpline曲线上的点Location
            FVector ClosestPointLocation = NavSpline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
            
            // ClosestPointLocation得到的NavSpline曲线上的点Direction
            FVector Direction = NavSpline->FindDirectionClosestToWorldLocation(ClosestPointLocation, ESplineCoordinateSpace::World);
           
            ControlledPawn->AddMovementInput(Direction);

            float DistanceAlongSpline = NavSpline->GetDistanceAlongSplineAtLocation(ClosestPointLocation, ESplineCoordinateSpace::World);
            if (DistanceAlongSpline >= (NavSpline->GetSplineLength() - AutoRunAcceptRadius)) bAutoRunning = false;
        }

    }
}

void AAuraPlayerController::ShowDamageText_Implementation(float DamageValue, AActor* Target)
{
    check(DamageTextComponentClass);
    if (IsValid(Target))
    {
        UDamageTextComponent* DamageTextComponent = NewObject<UDamageTextComponent>(Target, DamageTextComponentClass);
        DamageTextComponent->RegisterComponent();
        DamageTextComponent->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        DamageTextComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        DamageTextComponent->SetDamageText(DamageValue);
    }
  
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
    GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);

    LastActor = ThisActor;
    ThisActor = CursorHit.GetActor();

    if (LastActor != ThisActor)
    {
        if (ThisActor) ThisActor->HighlightActor();
        if (LastActor) LastActor->UnHighlightActor();
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
    FollowTime = 0.f;
    bAutoRunning = false;

    // Do Something To Cast AuraASC
    if (GetAuraASC() == nullptr) return;
    GetAuraASC()->OnAbilityInputPressed(Tag);
    
}

void AAuraPlayerController::OnAbilityInputReleased(FGameplayTag Tag)
{
    // 如果Tag不是Input_LMB
    if (!FGameplayTags::Get().InputTag_LMB.MatchesTagExact(Tag))
    {
        if (GetAuraASC() == nullptr) return;
        GetAuraASC()->OnAbilityInputPressed(Tag);
    }

    // 如果Tag是Input_LMB
    // 先告知ASC鼠标左键Released了
    if (GetAuraASC()) GetAuraASC()->OnAbilityInputReleased(Tag);

    // 如果没有指向目标 && 没有按住Shift，则进行移动
    if (!bTargeting && !bShiftDown)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn && FollowTime <= ShortThreshold)
        {
            if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination, ControlledPawn))
            {
                if (!NavPath->PathPoints.IsEmpty())
                {
                    NavSpline->ClearSplinePoints();
                    for (FVector& PathPoint : NavPath->PathPoints)
                    {
                        NavSpline->AddSplinePoint(PathPoint, ESplineCoordinateSpace::World);
                        DrawDebugSphere(GetWorld(), PathPoint, 5.f, 8, FColor::Green, false, 5.f);
                    }
                    bAutoRunning = true;
                    CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
                }
                
            }
        }
        FollowTime = 0.f;
        bTargeting = false;
    } 
}

void AAuraPlayerController::OnAbilityInputHeld(FGameplayTag Tag)
{
    // Input Tag不是Input.LMB时
    if (!FGameplayTags::Get().InputTag_LMB.MatchesTagExact(Tag))
    {
        if (GetAuraASC()) GetAuraASC()->OnAbilityInputHeld(Tag);
        return;
    }

    // Input Tag是Input.LMB时
    // 如果指向目标，释放技能；
    // 如果按住Shift，释放技能
    // 否则进行移动
    if (bTargeting || bShiftDown)
    {
        if (GetAuraASC()) GetAuraASC()->OnAbilityInputHeld(Tag);
        return;
    }
    else
    {
        FollowTime += GetWorld()->GetDeltaSeconds();
        if (CursorHit.bBlockingHit)
        {
            CachedDestination = CursorHit.ImpactPoint;

            if (APawn* ControlledPawn = GetPawn())
            {
                FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
                ControlledPawn->AddMovementInput(WorldDirection);
            }
        }
        
    }
}
