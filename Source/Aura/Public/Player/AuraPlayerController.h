// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class IEnemyInterface;
class UAuraAbilitySystemComponent;
class USplineComponent;
class UWidgetComponent;
struct FInputActionValue;


/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();

	// Client声明 -> 只在触发的某个客户端显示，不用Server给所有Client显示--Client
	// 造成伤害的非AI玩家都显示DamageText
	UFUNCTION(Client, Reliable)
	void ShowDamageText(float DamageValue, AActor* Target, bool bIsBlocked, bool bIsCritical);
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	
	void Move(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;

	bool bShiftDown = false;

	void OnShiftPressed() { bShiftDown = true; }
	void OnShiftReleased() { bShiftDown = false; }
	
	void CursorTrace();
	FHitResult CursorHit;

	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetAuraASC();

	void OnAbilityInputPressed(FGameplayTag Tag);
	void OnAbilityInputReleased(FGameplayTag Tag);
	void OnAbilityInputHeld(FGameplayTag Tag);


	bool bTargeting = false;
	bool bAutoRunning = false;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptRadius = 50.f;
	float FollowTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float ShortThreshold = 0.5;
	
	FVector CachedDestination = FVector::Zero();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> NavSpline;

	// Helper Function
	void AutoRun();

	UPROPERTY(EditDefaultsOnly, Category = "UI|DamageText")
	TSubclassOf<UWidgetComponent> DamageTextComponentClass;

};
