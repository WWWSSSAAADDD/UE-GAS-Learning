// Copyright 

#pragma once

#include "CoreMinimal.h"

/**
 * AuraGameplayTags
 * 
 * Singleton containing Native Gameplay Tags
 */
struct FGameplayTags
{
public:
	static FGameplayTags& Get() { static FGameplayTags singleton; return singleton; }
	static void InitializeNativeGameplayTags();
protected:

private:
};