#include "Lords_Frontiers/Public/UI/Widgets/PhasePanelWidget.h"

#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Localization/GameLocalization.h"
#include "UI/Widgets/StageProgressWidget.h"
#include "Waves/WaveManager.h"

void UPhasePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnWaveChanged.AddUniqueDynamic( this, &UPhasePanelWidget::HandleWaveChanged );
			gL->OnBuildTurnChanged.AddUniqueDynamic( this, &UPhasePanelWidget::HandleTurnChanged );
			gL->OnPhaseChanged.AddUniqueDynamic( this, &UPhasePanelWidget::HandlePhaseChanged );
		}
	}

	UpdateDayText();
}

void UPhasePanelWidget::NativeDestruct()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnWaveChanged.RemoveDynamic( this, &UPhasePanelWidget::HandleWaveChanged );
			gL->OnBuildTurnChanged.RemoveDynamic( this, &UPhasePanelWidget::HandleTurnChanged );
			gL->OnPhaseChanged.RemoveDynamic( this, &UPhasePanelWidget::HandlePhaseChanged );
		}
	}

	Super::NativeDestruct();
}

void UPhasePanelWidget::HandleWaveChanged( int32 CurrentWave, int32 TotalWaves )
{
	UpdateDayText();
}

void UPhasePanelWidget::HandleTurnChanged( int32 CurrentTurn, int32 MaxTurns )
{
	if ( StageProgressBar && MaxTurns > 0 )
	{
		const float progress = static_cast<float>( CurrentTurn - 1 ) / static_cast<float>( MaxTurns );
		StageProgressBar->SetTargetProgress( FMath::Clamp( progress, 0.0f, 1.0f ) );
	}
}

void UPhasePanelWidget::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	UpdateDayText();

	if ( StageProgressBar )
	{
		if ( NewPhase == EGameLoopPhase::Combat )
		{
			StageProgressBar->SetTargetProgress( 1.0f );
		}
		else if ( NewPhase != EGameLoopPhase::Building )
		{
			StageProgressBar->ResetProgressImmediate();
		}
	}
}

void UPhasePanelWidget::UpdateDayText()
{
	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return;
	}

	UGameLoopManager* gL = core->GetGameLoop();
	if ( !gL )
	{
		return;
	}

	const int32 wave = gL->GetCurrentWave();
	const int32 waveIndex = FMath::Max( 0, wave - 1 );

	const AWaveManager* waveManager = core->GetWaveManager();
	const bool bEndless =
	    waveManager && ( waveManager->IsEndlessRunActive() || waveManager->IsInfiniteWaveIndex( waveIndex ) );

	if ( TextDay )
	{
		TextDay->SetText( LF_LOC( "HUD.Day" ) );
	}

	if ( TextDayCurrent )
	{
		TextDayCurrent->SetText( FText::Format( FText::FromString( TEXT( " {0}" ) ), FText::AsNumber( wave ) ) );
	}

	if ( TextDayTotal )
	{
		if ( bEndless )
		{
			TextDayTotal->SetText( FText::GetEmpty() );
		}
		else
		{
			const int32 total = gL->GetTotalWaves();
			TextDayTotal->SetText( FText::Format( FText::FromString( TEXT( "/{0}" ) ), FText::AsNumber( total ) ) );
		}
	}
}
