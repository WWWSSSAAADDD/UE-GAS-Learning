// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "AuraInputConfig.h"
#include "AuraInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:

	// 一键给所有IA绑定Broadcast Tag的函数
	template<class UserName, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UAuraInputConfig* InputConfig, UserName* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleaseFunc, HeldFuncType HeldFunc);
};

template<class UserName, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
inline void UAuraInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, UserName* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (const FAuraInputAction& AuraIA : InputConfig->AbilityInputActions)
	{
		if (AuraIA.GameplayTag.IsValid() && IsValid(AuraIA.InputAction))
		{
			if (PressedFunc)
			{
				BindAction(AuraIA.InputAction, ETriggerEvent::Started, Object, PressedFunc, AuraIA.GameplayTag);
			}
			if (ReleasedFunc)
			{
				BindAction(AuraIA.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, AuraIA.GameplayTag);
			}
			if (HeldFunc)
			{
				BindAction(AuraIA.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, AuraIA.GameplayTag);
			}
		}
	}
}
