
// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CombatInterface.generated.h"

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual int32 GetPlayerLevel();

	// 规定Tag和AttackMontage的Tag一致
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetCombatSocketLocation(FGameplayTag MontageAttackTag);

	// 接口中的BlueprintImplementableEvent函数是“可选能力”，并不会保证所有实现了该接口的类都实现了这个函数
	// 使用时必须确保已经实现了
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UpdateFacingTarget(const FVector& TargetLocation);

	// BlueprintNativeEvenet：因为希望能在C++和蓝图都能重载该函数
	// 如果只有BlueprintCallable，没办法virtual重载
	// 如果有BlueprintNativeEvenet，会生成FunctionName_Implementation用于C++重载。
	// 蓝图可能会使用C++的重载
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void TempFunc() const;

	virtual void Died() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetAvatar();

  UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FTaggedMontage> GetAttackMontages() const;
};
