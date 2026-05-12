#include "Lords_Frontiers/Public/UI/Widgets/CombatTimerWidget.h"

#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/GameSessionController.h"

void UCombatTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( ButtonPause )
	{
		ButtonPause->OnClicked.AddDynamic( this, &UCombatTimerWidget::OnPauseClicked );
	}
	if ( ButtonPlay )
	{
		ButtonPlay->OnClicked.AddDynamic( this, &UCombatTimerWidget::OnPlayClicked );
	}
	if ( ButtonSpeedFast )
	{
		ButtonSpeedFast->OnClicked.AddDynamic( this, &UCombatTimerWidget::OnSpeedFastClicked );
	}
	if ( ButtonSpeedTurbo )
	{
		ButtonSpeedTurbo->OnClicked.AddDynamic( this, &UCombatTimerWidget::OnSpeedTurboClicked );
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.AddUniqueDynamic( this, &UCombatTimerWidget::HandlePhaseChanged );
			gL->OnCombatTimerUpdated.AddUniqueDynamic( this, &UCombatTimerWidget::HandleCombatTimer );
		}
	}

	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
		{
			session->OnSpeedChanged.AddUniqueDynamic( this, &UCombatTimerWidget::HandleSpeedChanged );
		}
	}

	UpdateTimerText( 0.0f );

	bool bIsCombat = false;
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			bIsCombat = ( gL->GetCurrentPhase() == EGameLoopPhase::Combat );
			if ( bIsCombat )
			{
				UpdateTimerText( gL->GetCombatTimeRemaining() );
			}
		}
	}

	float currentSpeed = PlaySpeed;
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
		{
			if ( bIsCombat )
			{
				currentSpeed = session->GetTimerScale();
			}
		}
	}
	UpdateActiveButtonVisuals( currentSpeed );

	SetCombatVisible( bIsCombat );
}

void UCombatTimerWidget::NativeDestruct()
{
	if ( ButtonPause )
	{
		ButtonPause->OnClicked.RemoveDynamic( this, &UCombatTimerWidget::OnPauseClicked );
	}
	if ( ButtonPlay )
	{
		ButtonPlay->OnClicked.RemoveDynamic( this, &UCombatTimerWidget::OnPlayClicked );
	}
	if ( ButtonSpeedFast )
	{
		ButtonSpeedFast->OnClicked.RemoveDynamic( this, &UCombatTimerWidget::OnSpeedFastClicked );
	}
	if ( ButtonSpeedTurbo )
	{
		ButtonSpeedTurbo->OnClicked.RemoveDynamic( this, &UCombatTimerWidget::OnSpeedTurboClicked );
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.RemoveDynamic( this, &UCombatTimerWidget::HandlePhaseChanged );
			gL->OnCombatTimerUpdated.RemoveDynamic( this, &UCombatTimerWidget::HandleCombatTimer );
		}
	}

	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
		{
			session->OnSpeedChanged.RemoveDynamic( this, &UCombatTimerWidget::HandleSpeedChanged );
		}
	}

	Super::NativeDestruct();
}

void UCombatTimerWidget::OnPauseClicked()
{
	ApplySpeed( PauseSpeed );
}

void UCombatTimerWidget::OnPlayClicked()
{
	ApplySpeed( PlaySpeed );
}

void UCombatTimerWidget::OnSpeedFastClicked()
{
	ApplySpeed( FastSpeed );
}

void UCombatTimerWidget::OnSpeedTurboClicked()
{
	ApplySpeed( TurboSpeed );
}

void UCombatTimerWidget::ApplySpeed( float Speed )
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
		{
			session->SetTimerScale( Speed );
		}
	}
}

void UCombatTimerWidget::HandleCombatTimer( float TimeRemaining, float TotalTime )
{
	UpdateTimerText( TimeRemaining );
}

void UCombatTimerWidget::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	const bool bIsCombat = ( NewPhase == EGameLoopPhase::Combat );
	SetCombatVisible( bIsCombat );

	if ( bIsCombat )
	{
		float currentSpeed = PlaySpeed;
		if ( UGameInstance* gi = GetGameInstance() )
		{
			if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
			{
				currentSpeed = session->GetTimerScale();
			}
		}
		UpdateActiveButtonVisuals( currentSpeed );
	}
}

void UCombatTimerWidget::HandleSpeedChanged( float NewSpeed )
{
	UpdateActiveButtonVisuals( NewSpeed );
}

void UCombatTimerWidget::UpdateTimerText( float TimeRemaining )
{
	if ( !TextTimer )
	{
		return;
	}

	const int32 totalSeconds = FMath::Max( 0, FMath::CeilToInt( TimeRemaining ) );
	const int32 minutes = totalSeconds / 60;
	const int32 seconds = totalSeconds % 60;
	TextTimer->SetText( FText::FromString( FString::Printf( TEXT( "%02d:%02d" ), minutes, seconds ) ) );
}

void UCombatTimerWidget::UpdateActiveButtonVisuals( float Speed )
{
	auto applyOpacity = [this]( UButton* button, bool bActive )
	{
		if ( !button )
		{
			return;
		}
		button->SetRenderOpacity( bActive ? ActiveButtonOpacity : InactiveButtonOpacity );
	};

	const bool bPause = FMath::IsNearlyEqual( Speed, PauseSpeed );
	const bool bPlay = !bPause && FMath::IsNearlyEqual( Speed, PlaySpeed );
	const bool bFast = !bPause && !bPlay && FMath::IsNearlyEqual( Speed, FastSpeed );
	const bool bTurbo = !bPause && !bPlay && !bFast && FMath::IsNearlyEqual( Speed, TurboSpeed );

	applyOpacity( ButtonPause, bPause );
	applyOpacity( ButtonPlay, bPlay );
	applyOpacity( ButtonSpeedFast, bFast );
	applyOpacity( ButtonSpeedTurbo, bTurbo );
}

void UCombatTimerWidget::SetCombatVisible( bool bVisible )
{
	const ESlateVisibility vis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	SetVisibility( vis );
}
