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

	SetVisibility( ESlateVisibility::Collapsed );
	if ( HealthBar )
	{
		HealthBar->SetPercent( 0.f );
	}
}

void UHealthBarWidget::NativeDestruct()
{
	Unbind( nullptr );
	Super::NativeDestruct();
}


void UHealthBarWidget::BindToActor( AActor* actor )
{
	// always unbind previous first
	Unbind( nullptr );

	if ( !actor )
	{
		BoundActor.Reset();
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	BoundActor = actor;

	SetVisibility( ESlateVisibility::Collapsed ); 

	// Try Unit
	if ( AUnit* unit = Cast<AUnit>( actor ) )
	{
		unit->OnUnitHealthChanged.AddDynamic( this, &UHealthBarWidget::OnActorHealthInt );

		actor->OnDestroyed.AddDynamic( this, &UHealthBarWidget::Unbind );

		LastKnownHealth_ = static_cast<int32>( unit->GetCurrentHealth() );
		LastDamageTimeSeconds_ = 0.f;                                  
		return;
	}

	// Try Building
	if ( ABuilding* building = Cast<ABuilding>( actor ) )
	{
		building->OnBuildingHealthChanged.AddDynamic( this, &UHealthBarWidget::OnActorHealthInt ); 
		actor->OnDestroyed.AddDynamic( this, &UHealthBarWidget::Unbind );               

		LastKnownHealth_ = static_cast<int32>( building->GetCurrentHealth() ); 
		LastDamageTimeSeconds_ = 0.f;                                   
		return;
	}

	BoundActor.Reset();
	SetVisibility( ESlateVisibility::Collapsed );
}

void UHealthBarWidget::Unbind( AActor* destroyedActor /*= nullptr*/ )
{
	AActor* actor = BoundActor.Get();
	if ( !actor )
		return;

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
	LastKnownHealth_ = -1;
	LastDamageTimeSeconds_ = 0.f;

	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ ); 
	}

	SetVisibility( ESlateVisibility::Collapsed );
}

void UHealthBarWidget::UpdateFromActor()
{
	if ( !BoundActor.IsValid() )
	{
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	int32 current = 0;
	int32 max = 0;

	if ( AUnit* unit = Cast<AUnit>( BoundActor.Get() ) )
	{
		// Replace with your unit getters if named differently
		current = static_cast<int32>( unit->GetCurrentHealth() );
		max = static_cast<int32>( unit->GetMaxHealth() );
	}
	else if ( ABuilding* building = Cast<ABuilding>( BoundActor.Get() ) )
	{
		current = static_cast<int32>( building->GetCurrentHealth() );
		max = static_cast<int32>( building->GetMaxHealth() );
	}
	else
	{
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	UpdateVisuals( current, max );
}

void UHealthBarWidget::OnActorHealthInt( int32 current, int32 max )
{
	UpdateVisuals( current, max );

	if ( bIsDead_ )
	{
		SetVisibility( ESlateVisibility::Collapsed );

		return;
	}
	if ( current <= 0 )
	{

		bIsDead_ = true;

		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
		}

		SetVisibility( ESlateVisibility::Collapsed );

		LastKnownHealth_ = current;

		return;
	}

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

		UE_LOG(
		    LogTemp, Log, TEXT( "HealthBarWidget: damage detected for %s -> show temporary (cur=%d)" ),
		    *GetNameSafe( BoundActor.Get() ), current
		);

		ShowTemporary();
	}
	else
	{
		LastKnownHealth_ = current;
	}
}
void UHealthBarWidget::UpdateVisuals( int32 current, int32 max )
{
	if ( !HealthBar )
		return;

	const float percent = ( max > 0 ) ? ( static_cast<float>( current ) / static_cast<float>( max ) ) : 0.f;
	HealthBar->SetPercent( FMath::Clamp( percent, 0.f, 1.f ) );
}

//  timer logic

void UHealthBarWidget::ShowTemporary()
{
	if ( bIsDead_ || bAutoHideSuspended_ )
	{
		UE_LOG(
		    LogTemp, Verbose, TEXT( "ShowTemporary skipped because bIsDead_==true for %s" ),
		    *GetNameSafe( BoundActor.Get() )
		);
		return;
	}
	SetVisibility( ESlateVisibility::Visible ); 

	if ( !GetWorld() )
		return;

	GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
	GetWorld()->GetTimerManager().SetTimer(
	    HideTimerHandle_, this, &UHealthBarWidget::HideIfIdle, VisibleOnDamageDuration, false
	); 
}

void UHealthBarWidget::HideIfIdle()
{
	if ( !GetWorld() )
		return;
	if ( bAutoHideSuspended_ )
	{
		return;
	}
	const float now = GetWorld()->GetTimeSeconds();
	// if no damage since timeout elapsed -> hide
	if ( now - LastDamageTimeSeconds_ >= VisibleOnDamageDuration )
	{
		SetVisibility( ESlateVisibility::Collapsed );
	}
}
void UHealthBarWidget::SuspendAutoHide( bool bSuspend )
{
	bAutoHideSuspended_ = bSuspend;
	if ( bSuspend )
	{
		if ( GetWorld() )
			GetWorld()->GetTimerManager().ClearTimer( HideTimerHandle_ );
	}
	else
	{
		// restart hide timer if recently had damage
		if ( LastDamageTimeSeconds_ > 0 && GetWorld() )
		{
			const float elapsed = GetWorld()->GetTimeSeconds() - LastDamageTimeSeconds_;
			const float remaining = FMath::Max( 0.f, VisibleOnDamageDuration - elapsed );
			if ( remaining > 0.f )
			{
				GetWorld()->GetTimerManager().SetTimer(
				    HideTimerHandle_, this, &UHealthBarWidget::HideIfIdle, remaining, false
				);
			}
			else
			{
				// hide immediately if time passed
				SetVisibility( ESlateVisibility::Collapsed );
			}
		}
	}
}
