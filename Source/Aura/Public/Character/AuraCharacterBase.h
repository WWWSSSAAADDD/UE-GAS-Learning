// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface  // 附带AbilitySystemComponent的实现接口以后便于操作Component
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAuraCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const noexcept override;
	UAttributeSet* GetAttributeSet() const noexcept { return AttributeSet; }

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Combat") // 所有成员指针都要有UPROPERTY宏，否则会被垃圾回收掉
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();
};
