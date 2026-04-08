// Copyright 


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (!OverlayWidgetController) {
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	/* 创建OverlayWidget、OverlayWidgetController 
	   给OverlayWidget绑定Controller
	   Display OverlayWidget
	*/

	checkf(OverlayWidgetClass, TEXT("未初始化OverlayWidgetClass，请在BP_AuraHUD选择"));
	checkf(OverlayWidgetControllerClass, TEXT("未初始化OverlayWidgetControllerClass，请在BP_AuraHUD选择")); 

	OverlayWidget = CreateWidget<UAuraUserWidget>(GetWorld(), OverlayWidgetClass);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

	/* Q:为什么不直接给OverlayWidgetController成员变量赋值？ */
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	OverlayWidget->SetWidgetController(WidgetController);

	WidgetController->BroadcastInitialValues();

	OverlayWidget->AddToViewport();

	
}
