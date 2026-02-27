#pragma once

#include "Components/WidgetComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HealthBarManager.generated.h"

/**
 * (Artyom)
 * Stores per-widget registration info
 */
class UHealthBarWidget;

UCLASS()
class LORDS_FRONTIERS_API AHealthBarManager : public AActor
{
	GENERATED_BODY()

public:
	AHealthBarManager();

	// Widget class (assign in Editor)
	UPROPERTY( EditAnywhere, Category = "HealthBar" )
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	// Default offset above actor origin when projecting to screen (world space)
	UPROPERTY( EditAnywhere, Category = "HealthBar" )
	FVector DefaultWorldOffset = FVector( 0.f, 0.f, 200.f );

	// If we want to auto-release widgets after some time (seconds). 0 = never.
	UPROPERTY( EditAnywhere, Category = "HealthBar" )
	float AutoReleaseSeconds = 4.0f;

	// Update frequency limit (seconds). If <= 0, update every tick.
	UPROPERTY( EditAnywhere, Category = "HealthBar|Performance" )
	float MinUpdateInterval = 0.01f; 

	// Thresholds to detect camera change (world units / degrees)
	UPROPERTY( EditAnywhere, Category = "HealthBar|Performance" )
	float CameraMoveThreshold = 1.0f; // world units

	UPROPERTY( EditAnywhere, Category = "HealthBar|Performance" )
	float CameraFOVThreshold = 0.25f; // degrees

	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void RegisterActor( AActor* Actor, const FVector& WorldOffset = FVector::ZeroVector );

	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void UnregisterActor( AActor* Actor );

	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void OnActorHealthChanged( AActor* Actor, int32 Current, int32 Max );

	// Show/hide all (for Alt press logic)
	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void ShowAllRegistered();

	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void HideAllRegistered();

	UFUNCTION( BlueprintCallable )
	void ShowActiveWidgets();

	UFUNCTION( BlueprintCallable )
	void HideActiveWidgets();

	UPROPERTY( EditAnywhere, Category = "HealthBar|Performance" )
	float ActorMoveThreshold = 10.0f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick( float delta ) override;

private:
	// Pool of widgets for reuse
	TArray<UHealthBarWidget*> WidgetPool_;

	// active actor -> widget map
	TMap<TWeakObjectPtr<AActor>, UHealthBarWidget*> ActiveWidgets_;

	// optional per-actor world offset (if user registered custom offsets)
	TMap<TWeakObjectPtr<AActor>, FVector> ActorOffsets_;

	// auto-release timers for active widgets
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ReleaseTimers_;

	TSet<TWeakObjectPtr<UHealthBarWidget>> ManagerForcedWidgets_;

	// last known world location for actors that have active widgets
	TMap<TWeakObjectPtr<AActor>, FVector> ActiveWidgetLastWorldPos_;

	void UpdatePositionForActor( AActor* actor );

	// camera state caching for change detection
	FVector LastCameraLocation_;
	FRotator LastCameraRotation_;
	float LastCameraFOV_ = 0.f;

	float LastCameraOrthoWidth_ = -1.f;

	UPROPERTY( EditAnywhere, Category = "HealthBar|Performance" )
	float CameraOrthoWidthThreshold = 10.0f;

	// accumulator to enforce MinUpdateInterval
	float UpdateAccumulator_ = 0.f;

	UHealthBarWidget* AcquireWidget();
	void ReleaseWidget( UHealthBarWidget* widget );

	FVector GetOffsetForActor( AActor* actor ) const;

	// Recompute screen positions for all active widgets
	void UpdateAllPositions();

	// Compute whether camera changed enough to require update
	bool HasCameraMovedSignificantly( APlayerController* pc ) const;
};