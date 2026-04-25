#include "Core/Subsystems/HealthBarPoolSubsystem/HealthBarPoolSubsystem.h"

#include "Building/Building.h"
#include "Camera/CameraComponent.h"
#include "Camera/StrategyCamera.h"
#include "Entity.h"
#include "UI/GameHUD.h"
#include "UI/HealthBar/HealthBarConfigDataAsset.h"
#include "UI/HealthBar/HealthBarWidget.h"
#include "Units/Unit.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

void UHealthBarPoolSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );
}

void UHealthBarPoolSubsystem::Deinitialize()
{
	for ( TPair<TWeakObjectPtr<AActor>, FActiveHealthBar>& pair : ActiveBars_ )
	{
		DetachBar( pair.Value );
	}
	ActiveBars_.Empty();
	Pools_.Empty();
	WarmedClasses_.Empty();
	LastDisplayedPercent_.Empty();

	Super::Deinitialize();
}

TStatId UHealthBarPoolSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( UHealthBarPoolSubsystem, STATGROUP_Tickables );
}

bool UHealthBarPoolSubsystem::IsTickable() const
{
	return ActiveBars_.Num() > 0;
}

int32 UHealthBarPoolSubsystem::GetPooledCount( TSubclassOf<UHealthBarWidget> widgetClass ) const
{
	if ( const FHealthBarPool* pool = Pools_.Find( widgetClass ) )
	{
		return pool->FreeWidgets.Num();
	}
	return 0;
}

void UHealthBarPoolSubsystem::PreWarm( TSubclassOf<UHealthBarWidget> widgetClass, int32 count )
{
	if ( !widgetClass || count <= 0 )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	FHealthBarPool& pool = Pools_.FindOrAdd( widgetClass );
	pool.FreeWidgets.Reserve( pool.FreeWidgets.Num() + count );

	for ( int32 i = 0; i < count; ++i )
	{
		UHealthBarWidget* widget = CreateWidget<UHealthBarWidget>( world, widgetClass );
		if ( widget )
		{
			widget->SetVisibility( ESlateVisibility::Collapsed );
			pool.FreeWidgets.Add( widget );
		}
	}

	WarmedClasses_.Add( widgetClass );
}

TSubclassOf<UHealthBarWidget> UHealthBarPoolSubsystem::ResolveWidgetClass(
    AActor* entity, UHealthBarConfigDataAsset* config
) const
{
	if ( !config )
	{
		return nullptr;
	}

	if ( const AUnit* unit = Cast<AUnit>( entity ) )
	{
		if ( const TSubclassOf<UHealthBarWidget>* override = config->EnemyOverrides.Find( unit->GetClass() ) )
		{
			if ( *override )
			{
				return *override;
			}
		}
	}
	else if ( const ABuilding* building = Cast<ABuilding>( entity ) )
	{
		if ( const TSubclassOf<UHealthBarWidget>* override = config->BuildingOverrides.Find( building->GetClass() ) )
		{
			if ( *override )
			{
				return *override;
			}
		}
	}

	return config->DefaultBarClass;
}

bool UHealthBarPoolSubsystem::IsBoss( AActor* entity ) const
{
	if ( const AUnit* unit = Cast<AUnit>( entity ) )
	{
		return unit->IsBoss();
	}
	return false;
}

void UHealthBarPoolSubsystem::ShowFor( AActor* entity, UHealthBarConfigDataAsset* config )
{
	if ( !IsValid( entity ) || !config )
	{
		return;
	}

	const TSubclassOf<UHealthBarWidget> widgetClass = ResolveWidgetClass( entity, config );
	if ( !widgetClass )
	{
		return;
	}

	if ( !WarmedClasses_.Contains( widgetClass ) )
	{
		CachedPoolSize_ = FMath::Max( 0, config->PoolSize_ );
		CachedHideDelay_ = config->HideDelay_;
		PreWarm( widgetClass, CachedPoolSize_ );
	}

	IEntity* entityInterface = Cast<IEntity>( entity );
	if ( !entityInterface )
	{
		return;
	}

	const FEntityStats& stats = entityInterface->Stats();
	const bool bIsBossEntity = IsBoss( entity );
	const bool bAtFullHealth = stats.IsAtFullHealth();

	if ( FActiveHealthBar* existing = ActiveBars_.Find( entity ) )
	{
		if ( !bAtFullHealth || bIsBossEntity )
		{
			existing->HideTimer = config->HideDelay_;
			existing->TouchOrder = ++TouchCounter_;
		}
		if ( existing->Widget )
		{
			existing->Widget->SetHealth( stats.Health(), stats.MaxHealth() );
		}
		return;
	}

	if ( bAtFullHealth && !bIsBossEntity )
	{
		return;
	}

	UHealthBarWidget* widget = AcquireWidget( widgetClass );
	if ( !widget )
	{
		return;
	}

	FActiveHealthBar bar;
	bar.Entity = entity;
	bar.Widget = widget;
	bar.Config = config;
	bar.bIsBoss = bIsBossEntity;
	bar.HideTimer = config->HideDelay_;
	bar.TouchOrder = ++TouchCounter_;

	if ( bar.bIsBoss )
	{
		if ( !AttachBossBar( bar, widget ) )
		{
			ReleaseWidget( widget );
			return;
		}
	}
	else
	{
		AttachWorldBar( bar, widget );
	}

	if ( const float* savedPercent = LastDisplayedPercent_.Find( entity ) )
	{
		widget->ResetTo( *savedPercent );
		LastDisplayedPercent_.Remove( entity );
	}
	else
	{
		widget->ResetToFull();
	}
	widget->SetHealth( stats.Health(), stats.MaxHealth() );

	widget->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
	widget->OnShow();

	ActiveBars_.Add( entity, bar );
}

void UHealthBarPoolSubsystem::HideFor( AActor* entity )
{
	if ( !entity )
	{
		return;
	}

	FActiveHealthBar bar;
	if ( !ActiveBars_.RemoveAndCopyValue( entity, bar ) )
	{
		return;
	}

	if ( bar.Widget )
	{
		bool bRememberPercent = true;
		if ( const IEntity* entityInterface = Cast<IEntity>( entity ) )
		{
			bRememberPercent = entityInterface->Stats().IsAlive();
		}
		if ( bRememberPercent )
		{
			LastDisplayedPercent_.Add( entity, bar.Widget->GetDisplayedPercent() );
		}
		else
		{
			LastDisplayedPercent_.Remove( entity );
		}
		bar.Widget->OnHide();
	}

	DetachBar( bar );

	if ( bar.Widget )
	{
		ReleaseWidget( bar.Widget );
	}
}

UHealthBarWidget* UHealthBarPoolSubsystem::AcquireWidget( TSubclassOf<UHealthBarWidget> widgetClass )
{
	FHealthBarPool& pool = Pools_.FindOrAdd( widgetClass );

	while ( pool.FreeWidgets.Num() > 0 )
	{
		UHealthBarWidget* candidate = pool.FreeWidgets.Pop( EAllowShrinking::No );
		if ( IsValid( candidate ) )
		{
			return candidate;
		}
	}

	if ( ActiveBars_.Num() >= CachedPoolSize_ && CachedPoolSize_ > 0 )
	{
		if ( UHealthBarWidget* recycled = EvictOldest( widgetClass ) )
		{
			return recycled;
		}
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return nullptr;
	}
	UHealthBarWidget* fresh = CreateWidget<UHealthBarWidget>( world, widgetClass );
	if ( fresh )
	{
		fresh->SetVisibility( ESlateVisibility::Collapsed );
	}
	return fresh;
}

UHealthBarWidget* UHealthBarPoolSubsystem::EvictOldest( TSubclassOf<UHealthBarWidget> widgetClass )
{
	TWeakObjectPtr<AActor> oldestKey;
	uint64 oldestOrder = TNumericLimits<uint64>::Max();

	for ( const TPair<TWeakObjectPtr<AActor>, FActiveHealthBar>& pair : ActiveBars_ )
	{
		if ( pair.Value.bIsBoss )
		{
			continue;
		}
		if ( pair.Value.TouchOrder < oldestOrder )
		{
			oldestOrder = pair.Value.TouchOrder;
			oldestKey = pair.Key;
		}
	}

	if ( !oldestKey.IsValid() )
	{
		return nullptr;
	}

	AActor* victimEntity = oldestKey.Get();
	HideFor( victimEntity );

	FHealthBarPool& pool = Pools_.FindOrAdd( widgetClass );
	while ( pool.FreeWidgets.Num() > 0 )
	{
		UHealthBarWidget* candidate = pool.FreeWidgets.Pop( EAllowShrinking::No );
		if ( IsValid( candidate ) )
		{
			return candidate;
		}
	}
	return nullptr;
}

void UHealthBarPoolSubsystem::ReleaseWidget( UHealthBarWidget* widget )
{
	if ( !IsValid( widget ) )
	{
		return;
	}

	widget->SetVisibility( ESlateVisibility::Collapsed );
	widget->SetRenderTransform( FWidgetTransform() );

	const TSubclassOf<UHealthBarWidget> cls = widget->GetClass();
	Pools_.FindOrAdd( cls ).FreeWidgets.Add( widget );
}

void UHealthBarPoolSubsystem::AttachWorldBar( FActiveHealthBar& bar, UHealthBarWidget* widget )
{
	AActor* entity = bar.Entity.Get();
	if ( !entity )
	{
		return;
	}

	UWidgetComponent* comp = NewObject<UWidgetComponent>( entity );
	comp->SetWidgetSpace( EWidgetSpace::Screen );
	comp->SetDrawAtDesiredSize( false );
	if ( bar.Config )
	{
		comp->SetDrawSize( bar.Config->WorldDrawSize_ );
		comp->SetRelativeLocation( FVector( 0.0f, 0.0f, bar.Config->WorldOffsetZ_ ) );
	}
	comp->SetWidget( widget );
	comp->SetupAttachment( entity->GetRootComponent() );
	comp->RegisterComponent();

	bar.WidgetComponent = comp;
}

bool UHealthBarPoolSubsystem::AttachBossBar( FActiveHealthBar& bar, UHealthBarWidget* widget )
{
	UGameHUDWidget* hud = ResolveHUD();
	if ( !hud )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UHealthBarPoolSubsystem::AttachBossBar: HUD widget not found yet" ) );
		return false;
	}
	return hud->AddBossBar( widget );
}

void UHealthBarPoolSubsystem::DetachBar( FActiveHealthBar& bar )
{
	if ( bar.bIsBoss )
	{
		if ( UGameHUDWidget* hud = ResolveHUD() )
		{
			hud->RemoveBossBar( bar.Widget );
		}
	}
	else if ( IsValid( bar.WidgetComponent ) )
	{
		bar.WidgetComponent->SetWidget( nullptr );
		bar.WidgetComponent->DestroyComponent();
	}
	bar.WidgetComponent = nullptr;
}

UGameHUDWidget* UHealthBarPoolSubsystem::ResolveHUD() const
{
	if ( HUDWeak_.IsValid() )
	{
		return HUDWeak_.Get();
	}

	TArray<UUserWidget*> foundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass( GetWorld(), foundWidgets, UGameHUDWidget::StaticClass(), false );
	if ( foundWidgets.Num() > 0 )
	{
		UGameHUDWidget* hud = Cast<UGameHUDWidget>( foundWidgets[0] );
		HUDWeak_ = hud;
		return hud;
	}
	return nullptr;
}

float UHealthBarPoolSubsystem::GetCameraZoomAlpha() const
{
	APawn* playerPawn = UGameplayStatics::GetPlayerPawn( GetWorld(), 0 );
	const AStrategyCamera* strategyCam = Cast<AStrategyCamera>( playerPawn );
	if ( !strategyCam || !strategyCam->Camera )
	{
		return 0.5f;
	}

	float currentZoom = 0.0f;
	if ( strategyCam->Camera->ProjectionMode == ECameraProjectionMode::Orthographic )
	{
		currentZoom = strategyCam->Camera->OrthoWidth;
	}
	else if ( strategyCam->SpringArm )
	{
		currentZoom = strategyCam->SpringArm->TargetArmLength;
	}
	else
	{
		return 0.5f;
	}

	const float minZoom = FMath::Max( 1.0f, strategyCam->MinZoom() );
	const float maxZoom = FMath::Max( minZoom + 1.0f, strategyCam->MaxZoom() );
	const float safeZoom = FMath::Clamp( currentZoom, minZoom, maxZoom );

	const float invCurrent = 1.0f / safeZoom;
	const float invMin = 1.0f / minZoom;
	const float invMax = 1.0f / maxZoom;

	const float closeness = ( invCurrent - invMax ) / ( invMin - invMax );

	return FMath::Clamp( 1.0f - closeness, 0.0f, 1.0f );
}

void UHealthBarPoolSubsystem::Tick( float deltaTime )
{
	const float zoomAlpha = GetCameraZoomAlpha();

	TArray<AActor*> expired;
	for ( TPair<TWeakObjectPtr<AActor>, FActiveHealthBar>& pair : ActiveBars_ )
	{
		FActiveHealthBar& bar = pair.Value;
		if ( !bar.Entity.IsValid() )
		{
			expired.Add( pair.Key.Get() );
			continue;
		}

		if ( bar.Widget )
		{
			bar.Widget->TickAnim( deltaTime );
			bar.Widget->ApplyCameraScale( zoomAlpha );
		}

		if ( !bar.bIsBoss )
		{
			bar.HideTimer -= deltaTime;
			if ( bar.HideTimer <= 0.0f )
			{
				expired.Add( pair.Key.Get() );
			}
		}
	}

	for ( AActor* entity : expired )
	{
		HideFor( entity );
	}
}
