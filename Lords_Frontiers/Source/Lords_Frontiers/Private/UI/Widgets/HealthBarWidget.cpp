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

	if ( AUnit* unit = Cast<AUnit>( actor ) )
	{
		unit->OnUnitHealthChanged.AddDynamic( this, &UHealthBarWidget::OnActorHealthInt );

		actor->OnDestroyed.AddDynamic( this, &UHealthBarWidget::Unbind );

		UpdateFromActor();
		return;
	}

	if ( ABuilding* building = Cast<ABuilding>( actor ) )
	{
		building->OnBuildingHealthChanged.AddDynamic( this, &UHealthBarWidget::OnActorHealthInt );
		actor->OnDestroyed.AddDynamic( this, &UHealthBarWidget::Unbind );

		UpdateFromActor();
		return;
	}

	BoundActor.Reset();
	SetVisibility( ESlateVisibility::Collapsed );
}

void UHealthBarWidget::Unbind( AActor* destroyedActor )
{
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

	if ( LastKnownHealth_ == -1 )
	{
		LastKnownHealth_ = cur;
	}

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
		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
		}
	}
	else
	{
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
	UpdateVisuals( current, max );

	if ( LastKnownHealth_ == -1 )
	{
		LastKnownHealth_ = current;
		return;
	}

	if ( current < LastKnownHealth_ )
	{
		LastKnownHealth_ = current;
		if ( GetWorld() )
		{
			LastDamageTimeSeconds_ = GetWorld()->GetTimeSeconds();
		}

		ShowTemporary();
	}
	else
	{
		LastKnownHealth_ = current;
	}

	// If actor died, hide and cancel timers
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
		SetVisibility( ESlateVisibility::Visible );
		return;
	}

	SetVisibility( ESlateVisibility::Visible );

	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
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
		// schedule again
		const float remaining = VisibleOnDamageDuration - ( now - LastDamageTimeSeconds_ );
		GetWorld()->GetTimerManager().SetTimer(
		    HideTimerHandle_, this, &UHealthBarWidget::HideIfIdle, remaining, false
		);
	}
}