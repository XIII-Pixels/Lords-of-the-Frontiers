#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "CoreMinimal.h"

#include "HealthBarWidget.generated.h"

class AActor;
class AUnit;
class ABuilding;

/**
* (Artyom)
 * UHealthBarWidget
 */
UCLASS()
class LORDS_FRONTIERS_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Health" )
	void BindToActor( AActor* Actor );

	UFUNCTION()
	void Unbind( AActor* DestroyedActor = nullptr );

	UFUNCTION( BlueprintCallable, Category = "Health" )
	void UpdateFromActor();

	UFUNCTION( BlueprintCallable, Category = "Health" )
	void SuspendAutoHide( bool bSuspend );
	
	// show temporarily (and restart timer)
	void ShowTemporary();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Bound handler signature — must match delegate on AUnit/ABuilding
	UFUNCTION()
	void OnActorHealthInt( int32 Current, int32 Max );

	// update visuals
	void UpdateVisuals( int32 Current, int32 Max );

	// called by timer to attempt hide
	void HideIfIdle();

	// widgets from UMG (must exist in the UMG widget)
	UPROPERTY( meta = ( BindWidget ) )
	UProgressBar* HealthBar;

	// how long to keep visible after receiving damage
	UPROPERTY( EditAnywhere, Category = "Settings|Health" )
	float VisibleOnDamageDuration = 3.0f;

private:
	// timer handle for auto-hide
	FTimerHandle HideTimerHandle_;

	// time of last damage (world seconds)
	float LastDamageTimeSeconds_ = 0.0f;

	// last known health to detect damage; -1 uninitialized
	int32 LastKnownHealth_ = -1;

	// bound actor
	TWeakObjectPtr<AActor> BoundActor;

	// is actor dead (health <= 0)
	bool bIsDead_ = false;

	// if true, don't auto-hide (e.g. Alt is held)
	bool bAutoHideSuspended_ = false;
};