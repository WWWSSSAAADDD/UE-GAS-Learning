
// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"


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

	virtual FVector GetWeaponTipLocation();

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

	virtual void Died() = 0;
};
