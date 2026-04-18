// Copyright 


#include "AuraAssetManager.h"
#include "AuraGameplayTags.h"

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FGameplayTags::InitializeNativeGameplayTags();
}
