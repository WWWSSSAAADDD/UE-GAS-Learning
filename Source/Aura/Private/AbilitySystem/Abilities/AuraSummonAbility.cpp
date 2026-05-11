// Copyright 


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	int Index = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Index];
}

TArray<FVector> UAuraSummonAbility::GetSummonLocations()
{
	TArray<FVector> SummonLocations;
	
	float HalfAngleDegree = AngleDegree / 2;
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	FVector Origin = Avatar->GetActorLocation();
	FVector ActorForward = Avatar->GetActorForwardVector();

	FVector LeftMostDirection = ActorForward.RotateAngleAxis(-HalfAngleDegree, FVector(0, 0, 1));
	float IntervalDegree = AngleDegree / (SummonNums - 1);
	for (int i = 0; i < SummonNums; i++)
	{
		FVector Direction = LeftMostDirection.RotateAngleAxis(IntervalDegree * i, FVector(0, 0, 1));
		double RandomDistance = FMath::RandRange(MinDistance, MaxDistance);
		FVector SummonLocation = Origin + Direction * RandomDistance;
		
		FHitResult Hit;
		Avatar->GetWorld()->LineTraceSingleByChannel(Hit, SummonLocation + FVector(0, 0, 400.f), SummonLocation + FVector(0, 0, -400.f), ECC_WorldStatic);
		SummonLocations.Add(Hit.Location);
	}

	return SummonLocations;
}
