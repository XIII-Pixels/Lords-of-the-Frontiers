#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CardFeedbackPopup.generated.h"

class UWidgetComponent;
class UCardFeedbackWidget;
class UTexture2D;

UENUM()
enum class ECardFeedbackPopupMode : uint8
{
	Popup,
	Sticky
};

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ACardFeedbackPopup : public AActor
{
	GENERATED_BODY()

public:
	ACardFeedbackPopup();

	void ActivatePopup(
		AActor* owner, UTexture2D* icon,
		float durationSeconds, float floatHeight, float baseHeightOffset );

	void ActivateSticky( AActor* owner, UTexture2D* icon, float baseHeightOffset );

	void DeactivateToPool();

	bool IsActive() const
	{
		return bIsActive_;
	}

	DECLARE_MULTICAST_DELEGATE_OneParam( FOnPopupFinished, ACardFeedbackPopup* );
	FOnPopupFinished OnPopupFinished;

protected:
	virtual void BeginPlay() override;
	virtual void Tick( float deltaTime ) override;

	void ApplyIcon( UTexture2D* icon );
	void SetDormant( bool bDormant );

	UPROPERTY( VisibleAnywhere, Category = "Settings|Feedback" )
	TObjectPtr<UWidgetComponent> WidgetComponent_;

	UPROPERTY( EditAnywhere, Category = "Settings|Feedback" )
	TSubclassOf<UCardFeedbackWidget> WidgetClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Feedback" )
	FVector2D WidgetDrawSize_ = FVector2D( 128.f, 128.f );

	UPROPERTY()
	TObjectPtr<UCardFeedbackWidget> WidgetInstance_;

	ECardFeedbackPopupMode Mode_ = ECardFeedbackPopupMode::Popup;
	bool bIsActive_ = false;

	float Duration_ = 1.0f;
	float Elapsed_ = 0.0f;
	float FloatHeight_ = 120.f;
	float BaseHeightOffset_ = 0.f;

	TWeakObjectPtr<AActor> OwnerActor_;
};
