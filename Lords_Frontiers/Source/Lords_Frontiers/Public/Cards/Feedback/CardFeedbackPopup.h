#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CardFeedbackPopup.generated.h"

class UWidgetComponent;
class UCardFeedbackWidget;
class UTexture2D;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ACardFeedbackPopup : public AActor
{
	GENERATED_BODY()

public:
	ACardFeedbackPopup();

	void Initialize( AActor* owner, UTexture2D* icon, float durationSeconds, float floatHeight );

protected:
	virtual void BeginPlay() override;
	virtual void Tick( float deltaTime ) override;

	UPROPERTY( VisibleAnywhere, Category = "Feedback" )
	TObjectPtr<UWidgetComponent> WidgetComponent_;

	UPROPERTY( EditAnywhere, Category = "Feedback" )
	TSubclassOf<UCardFeedbackWidget> WidgetClass_;

	UPROPERTY( EditAnywhere, Category = "Feedback" )
	FVector2D WidgetDrawSize_ = FVector2D( 128.f, 128.f );

	UPROPERTY()
	TObjectPtr<UCardFeedbackWidget> WidgetInstance_;

	float Duration_ = 1.0f;
	float Elapsed_ = 0.0f;
	float FloatHeight_ = 120.f;

	TWeakObjectPtr<AActor> OwnerActor_;
};
