#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/WeakObjectPtr.h"

#include "HealthBarPoolSubsystem.generated.h"

class UHealthBarWidget;
class UHealthBarConfigDataAsset;
class UWidgetComponent;
class UGameHUDWidget;
class AActor;
class AUnit;
class ABuilding;

USTRUCT()
struct FHealthBarPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UHealthBarWidget>> FreeWidgets;
};

USTRUCT()
struct FActiveHealthBar
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Entity;

	UPROPERTY()
	TObjectPtr<UHealthBarWidget> Widget;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY()
	TObjectPtr<UHealthBarConfigDataAsset> Config;

	bool bIsBoss = false;

	float HideTimer = 0.0f;

	uint64 TouchOrder = 0;
};

UCLASS()
class LORDS_FRONTIERS_API UHealthBarPoolSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	virtual void Tick( float deltaTime ) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}
	virtual bool IsTickable() const override;

	void PreWarm( TSubclassOf<UHealthBarWidget> widgetClass, int32 count );

	void ShowFor( AActor* entity, UHealthBarConfigDataAsset* config );

	void HideFor( AActor* entity );

	int32 GetActiveCount() const
	{
		return ActiveBars_.Num();
	}

	int32 GetPooledCount( TSubclassOf<UHealthBarWidget> widgetClass ) const;

private:

	UHealthBarWidget* AcquireWidget( TSubclassOf<UHealthBarWidget> widgetClass );

	void ReleaseWidget( UHealthBarWidget* widget );

	TSubclassOf<UHealthBarWidget> ResolveWidgetClass( AActor* entity, UHealthBarConfigDataAsset* config ) const;

	bool IsBoss( AActor* entity ) const;

	UHealthBarWidget* EvictOldest( TSubclassOf<UHealthBarWidget> widgetClass );

	void AttachWorldBar( FActiveHealthBar& bar, UHealthBarWidget* widget );

	void AttachBossBar( FActiveHealthBar& bar, UHealthBarWidget* widget );

	void DetachBar( FActiveHealthBar& bar );

	float GetCameraZoomAlpha() const;

	UGameHUDWidget* ResolveHUD() const;

	UPROPERTY()
	TMap<TSubclassOf<UHealthBarWidget>, FHealthBarPool> Pools_;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FActiveHealthBar> ActiveBars_;

	uint64 TouchCounter_ = 0;

	float CachedHideDelay_ = 3.0f;
	int32 CachedPoolSize_ = 64;

	TSet<TSubclassOf<UHealthBarWidget>> WarmedClasses_;

	mutable TWeakObjectPtr<UGameHUDWidget> HUDWeak_;
};
