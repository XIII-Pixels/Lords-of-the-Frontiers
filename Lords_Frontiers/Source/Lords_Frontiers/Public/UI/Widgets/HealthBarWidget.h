#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "HealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class AActor;

class UProgressBar;

/**
 * (Artyom)
 * Health bar widget attached to actors (units / buildings).
 * Shows only a progress bar. Appears on damage and hides after timeout.
 */
UCLASS()
class LORDS_FRONTIERS_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Health" )
	void BindToActor( AActor* actor );

	UFUNCTION()
	void Unbind( AActor* destroyedActor = nullptr );

	UFUNCTION( BlueprintCallable, Category = "Health" )
	void UpdateFromActor();

	UFUNCTION( BlueprintCallable )
	void SuspendAutoHide( bool bSuspend );

protected:
	virtual void NativeConstruct() override; 
	virtual void NativeDestruct() override;

	// Health change handler
	UFUNCTION()
	void OnActorHealthInt( int32 current, int32 max );

	// Update progress bar
	void UpdateVisuals( int32 current, int32 max );

	// Show widget for VisibleOnDamageDuration seconds 
	void ShowTemporary();

	// hide widget if idle long enough
	void HideIfIdle();

	// Widget bind (progress bar )
	UPROPERTY( meta = ( BindWidget ) )
	UProgressBar* HealthBar;

	UPROPERTY( EditAnywhere, Category = "Settings|Health" )
	float VisibleOnDamageDuration = 3.0f;

private:
	// Timer handle for hide timer
	FTimerHandle HideTimerHandle_;

	// Time of last damage (world seconds)
	float LastDamageTimeSeconds_ = 0.0f;

	// last known health to detect damage. -1 = uninitialized
	int32 LastKnownHealth_ = -1;

	TWeakObjectPtr<AActor> BoundActor;

	bool bIsDead_ = false;

	bool bAutoHideSuspended_ = false;
};