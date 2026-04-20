// Copyright 


#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AttributeSet.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{	
	UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);

	for (const auto& TagToAttribute : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(TagToAttribute.Key, TagToAttribute.Value());
	}

}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);
	// bind callback to ASC OnAttributeChangedDelegate
	for (const auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& Tag, const FGameplayAttribute& Attribute)
{
	UAttributeInfo* AttributeInfoAsset = CastChecked<UAttributeInfo>(AttributeInfo);

	FAuraAttributeInfo Info = AttributeInfoAsset->FindAttributeInfoForTag(Tag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
