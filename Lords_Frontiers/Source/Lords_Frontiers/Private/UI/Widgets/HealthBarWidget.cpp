#include "Lords_Frontiers/Public/UI/Widgets/HealthBarWidget.h"

#include "Building/Building.h"
#include "Units/Unit.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// initial visuals
	if ( HealthBar )
	{
		HealthBar->SetPercent( 0.f );
	}

	SetVisibility( ESlateVisibility::Collapsed );
}

void UHealthBarWidget::NativeDestruct()
{
	Unbind();
	Super::NativeDestruct();
}

void UHealthBarWidget::BindToActor( AActor* actor )
{
	// safe unbind previous
	Unbind();

	if ( !actor )
	{
		BoundActor.Reset();
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	BoundActor = actor;
	bIsDead_ = false;
	LastKnownHealth_ = -1;

	// subscribe to actor health delegates if actor exposes them
	if ( AUnit* unit = Cast<AUnit>( actor ) )
	{
		// assuming AUnit has: UPROPERTY(BlueprintAssignable) FOnUnitHealthChanged OnUnitHealthChanged;
		// and signature (int32 current, int32 max)
		unit->OnUnitHealthChanged.AddDynamic( this, &UHealthBarWidget::OnActorHealthInt );

		// cleanup on actor destroy
		actor->OnDestroyed.AddDynamic( this, &UHealthBarWidget::Unbind );

		// initialize immediately
		UpdateFromActor();
		return;
	}

	if ( ABuilding* building = Cast<ABuilding>( actor ) )
	{
		// assuming ABuilding has OnBuildingHealthChanged
		building->OnBuildingHealthChanged.AddDynamic( this, &UHealthBarWidget::OnActorHealthInt );
		actor->OnDestroyed.AddDynamic( this, &UHealthBarWidget::Unbind );

		UpdateFromActor();
		return;
	}

	// otherwise not supported actor — hide
	BoundActor.Reset();
	SetVisibility( ESlateVisibility::Collapsed );
}

void UHealthBarWidget::Unbind( AActor* destroyedActor )
{
	// stop timer
	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
	}

	AActor* actor = BoundActor.Get();
	if ( !actor )
	{
		BoundActor.Reset();
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	if ( AUnit* unit = Cast<AUnit>( actor ) )
	{
		// remove delegate (if bound)
		unit->OnUnitHealthChanged.RemoveDynamic( this, &UHealthBarWidget::OnActorHealthInt );
		actor->OnDestroyed.RemoveDynamic( this, &UHealthBarWidget::Unbind );
	}
	else if ( ABuilding* building = Cast<ABuilding>( actor ) )
	{
		building->OnBuildingHealthChanged.RemoveDynamic( this, &UHealthBarWidget::OnActorHealthInt );
		actor->OnDestroyed.RemoveDynamic( this, &UHealthBarWidget::Unbind );
	}

	BoundActor.Reset();
	SetVisibility( ESlateVisibility::Collapsed );
	LastKnownHealth_ = -1;
	bIsDead_ = false;
	bAutoHideSuspended_ = false;
}

void UHealthBarWidget::UpdateFromActor()
{
	AActor* actor = BoundActor.Get();
	if ( !actor )
	{
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	int32 cur = 0, max = 0;
	if ( AUnit* unit = Cast<AUnit>( actor ) )
	{
		cur = static_cast<int32>( unit->GetCurrentHealth() );
		max = static_cast<int32>( unit->GetMaxHealth() );
	}
	else if ( ABuilding* building = Cast<ABuilding>( actor ) )
	{
		cur = static_cast<int32>( building->GetCurrentHealth() );
		max = static_cast<int32>( building->GetMaxHealth() );
	}
	else
	{
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	UpdateVisuals( cur, max );

	// Initialize LastKnownHealth_ if wasn't initialized yet
	if ( LastKnownHealth_ == -1 )
	{
		LastKnownHealth_ = cur;
	}

	// if dead, ensure hidden
	if ( cur <= 0 )
	{
		bIsDead_ = true;
		SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UHealthBarWidget::SuspendAutoHide( bool bSuspend )
{
	bAutoHideSuspended_ = bSuspend;

	if ( bAutoHideSuspended_ )
	{
		// if suspending, clear any hide timer and ensure visible if actor is valid
		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
		}
	}
	else
	{
		// resumed: if there was recent damage, schedule hide accordingly
		if ( BoundActor.IsValid() && LastDamageTimeSeconds_ > 0.f && GetWorld() )
		{
			const float now = GetWorld()->GetTimeSeconds();
			const float elapsed = now - LastDamageTimeSeconds_;
			const float remaining = FMath::Max( 0.f, VisibleOnDamageDuration - elapsed );
			if ( remaining > 0.f )
			{
				GetWorld()->GetTimerManager().SetTimer(
				    HideTimerHandle_, this, &UHealthBarWidget::HideIfIdle, remaining, false
				);
			}
			else
			{
				// already expired -> hide
				SetVisibility( ESlateVisibility::Collapsed );
			}
		}
	}
}

void UHealthBarWidget::OnActorHealthInt( int32 current, int32 max )
{
	// Debug log (optional)
	UE_LOG(
	    LogTemp, Verbose,
	    TEXT( "HealthBarWidget::OnActorHealthInt Actor=%s Current=%d Max=%d LastKnown=%d bIsDead=%d" ),
	    *GetNameSafe( BoundActor.Get() ), current, max, LastKnownHealth_, bIsDead_ ? 1 : 0
	);

	// Update visuals always
	UpdateVisuals( current, max );

	// Initialize LastKnownHealth_ if uninitialized
	if ( LastKnownHealth_ == -1 )
	{
		LastKnownHealth_ = current;
		return;
	}

	// If we've taken damage (current < last known)
	if ( current < LastKnownHealth_ )
	{
		LastKnownHealth_ = current;
		if ( GetWorld() )
		{
			LastDamageTimeSeconds_ = GetWorld()->GetTimeSeconds();
		}

		// show bar temporarily (unless suspended)
		ShowTemporary();
	}
	else
	{
		// no damage (healing or same) - update cache
		LastKnownHealth_ = current;
	}

	// If actor died, hide immediately and cancel timers
	if ( current <= 0 )
	{
		bIsDead_ = true;
		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
		}
		SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UHealthBarWidget::UpdateVisuals( int32 current, int32 max )
{
	if ( !HealthBar )
		return;

	const float Percent = ( max > 0 ) ? ( float( current ) / float( max ) ) : 0.f;
	HealthBar->SetPercent( FMath::Clamp( Percent, 0.f, 1.f ) );
}

void UHealthBarWidget::ShowTemporary()
{
	if ( bAutoHideSuspended_ )
	{
		// show but don't start hide timer
		SetVisibility( ESlateVisibility::Visible );
		return;
	}

	SetVisibility( ESlateVisibility::Visible );

	if ( GetWorld() )
	{
		// clear previous timer
		GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
		// schedule hide
		GetWorld()->GetTimerManager().SetTimer(
		    HideTimerHandle_, this, &UHealthBarWidget::HideIfIdle, VisibleOnDamageDuration, false
		);
	}
}

void UHealthBarWidget::HideIfIdle()
{
	if ( bAutoHideSuspended_ )
		return;

	// only hide if enough time passed since last damage
	if ( !GetWorld() )
		return;

	const float now = GetWorld()->GetTimeSeconds();
	if ( LastDamageTimeSeconds_ <= 0.f || ( now - LastDamageTimeSeconds_ ) >= VisibleOnDamageDuration )
	{
		SetVisibility( ESlateVisibility::Collapsed );
	}
	else
	{
		// schedule again for remaining time
		const float remaining = VisibleOnDamageDuration - ( now - LastDamageTimeSeconds_ );
		GetWorld()->GetTimerManager().SetTimer(
		    HideTimerHandle_, this, &UHealthBarWidget::HideIfIdle, remaining, false
		);
	}
}