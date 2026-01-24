// DebugUIWidget.h

#pragma once

#include "AI/Path/Path.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CoreMinimal.h"

#include "DebugPathUIWidget.generated.h"

class APathTargetPoint;
class UPathPointsManager;
class UButton;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UDebugPathUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button1;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button2;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnButton1Clicked();

	UFUNCTION()
	void OnButton2Clicked();

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Path" )
	TSubclassOf<APathTargetPoint> PathTargetPointClass_;

	UPROPERTY()
	TObjectPtr<UPath> Path_;

	UPROPERTY()
	TObjectPtr<UPathPointsManager> PathPointsManager_;
};
