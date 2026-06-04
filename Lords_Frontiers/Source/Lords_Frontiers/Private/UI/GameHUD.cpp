#include "Lords_Frontiers/Public/UI/GameHUD.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Building/DefensiveBuilding.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Debug/DebugPlayerController.h"
#include "Localization/GameLocalization.h"
#include "Resources/ResourceManager.h"
#include "UI/CursorAnim/CursorAnimationConfig.h"
#include "UI/CursorAnim/CursorAnimationSubsystem.h"
#include "UI/HealthBar/HealthBarWidget.h"
#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraZoomUtils.h"
#include "Camera/StrategyCamera.h"
#include "Components/GridPanel.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( UCursorAnimationSubsystem* cursorAnimSubsystem = UCursorAnimationSubsystem::Get( this ) )
	{
		cursorAnimSubsystem->SetConfig( CursorAnimConfig );
	}

	if ( ButtonRelocateBuilding )
	{
		ButtonRelocateBuilding->OnClicked.AddDynamic( this, &UGameHUDWidget::OnRelocateBuildingClicked );
		ButtonRelocateBuilding->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverRelocateBuilding );
	}
	if ( ButtonRemoveBuilding )
	{
		ButtonRemoveBuilding->OnClicked.AddDynamic( this, &UGameHUDWidget::OnRemoveBuildingClicked );
		ButtonRemoveBuilding->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverRemoveBuilding );
	}
	if ( ButtonEndTurn )
	{
		ButtonEndTurn->OnClicked.AddDynamic( this, &UGameHUDWidget::OnEndTurnClicked );
		ButtonEndTurn->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverEndTurn );
	}

	if ( ABuildManager* buildManager =
	         Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) ) )
	{
		buildManager->OnBonusPreviewUpdated.AddDynamic( this, &UGameHUDWidget::HandleBonusPreviewUpdated );
		buildManager->OnPlacingStarted.AddUniqueDynamic( this, &UGameHUDWidget::HandlePlacingStarted );
		buildManager->OnPlacingCancelled.AddUniqueDynamic( this, &UGameHUDWidget::HandlePlacingEnded );
	}

	if ( ConstructionPanel )
	{
		ConstructionPanel->SetVisibility( ESlateVisibility::Collapsed );
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.AddDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			gL->OnBuildTurnChanged.AddDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
			{
				session->OnGameEndDelegate.AddUniqueDynamic( this, &UGameHUDWidget::HandleGameEnded );
			}
		}
	}

	// Sound
	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->RegisterObject( this );
			}
		}
	}

	if ( IsValid( EnemyTooltipClass ) && !IsValid( ActiveEnemyTooltip ) )
	{
		ActiveEnemyTooltip = CreateWidget<UEnemyTooltipWidget>( this, EnemyTooltipClass );
		if ( IsValid( ActiveEnemyTooltip ) )
		{
			ActiveEnemyTooltip->AddToViewport( 99 );
			ActiveEnemyTooltip->ForceHide();
		}
	}

	UpdateStatusText();
	UpdateButtonVisibility();

	if ( IsValid( WaveInfoPanel ) )
	{
		ActiveWavePanel = WaveInfoPanel;
	}
	else if ( IsValid( WavePanelClass ) && !IsValid( ActiveWavePanel ) )
	{
		ActiveWavePanel = CreateWidget<UWaveInfoPanelWidget>( this, WavePanelClass );
		if ( IsValid( ActiveWavePanel ) )
		{
			ActiveWavePanel->AddToViewport( -1 );
		}
	}
	UpdateWaveInfo();
}

void UGameHUDWidget::NativeDestruct()
{
	if ( ButtonRelocateBuilding )
	{
		ButtonRelocateBuilding->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnRelocateBuildingClicked );
		ButtonRelocateBuilding->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverRelocateBuilding );
	}
	if ( ButtonRemoveBuilding )
	{
		ButtonRemoveBuilding->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnRemoveBuildingClicked );
		ButtonRemoveBuilding->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverRemoveBuilding );
	}

	if ( ButtonEndTurn )
	{
		ButtonEndTurn->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnEndTurnClicked );
		ButtonEndTurn->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverEndTurn );
	}

	if ( ABuildManager* buildManager =
	         Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) ) )
	{
		buildManager->OnBonusPreviewUpdated.RemoveDynamic( this, &UGameHUDWidget::HandleBonusPreviewUpdated );
		buildManager->OnPlacingStarted.RemoveDynamic( this, &UGameHUDWidget::HandlePlacingStarted );
		buildManager->OnPlacingCancelled.RemoveDynamic( this, &UGameHUDWidget::HandlePlacingEnded );
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.RemoveDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			gL->OnBuildTurnChanged.RemoveDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
			{
				session->OnGameEndDelegate.RemoveDynamic( this, &UGameHUDWidget::HandleGameEnded );
			}
		}
	}

	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->UnregisterObject( this );
			}
		}
	}

	Super::NativeDestruct();
}

void UGameHUDWidget::HandleTurnChanged( int32 CurrentTurn, int32 MaxTurns )
{
	UpdateStatusText();
}

void UGameHUDWidget::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	UpdateStatusText();
	UpdateButtonVisibility();

	UpdateExtraButtonsVisibility();

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( AWaveManager* waveManager = core->GetWaveManager() )
		{
			if ( NewPhase == EGameLoopPhase::Combat )
			{
				waveManager->OnWaveEnemiesUpdated.AddUniqueDynamic( this, &UGameHUDWidget::UpdateWaveInfo );
				UpdateWaveInfo();
			}
			else if ( NewPhase == EGameLoopPhase::Building )
			{
				waveManager->OnWaveEnemiesUpdated.RemoveDynamic( this, &UGameHUDWidget::UpdateWaveInfo );
				UpdateWaveInfo();
			}
		}
	}
}

void UGameHUDWidget::HandlePlacingStarted()
{
	if ( ConstructionPanel )
	{
		ConstructionPanel->SetPanelVisible( true );
	}
}

void UGameHUDWidget::HandlePlacingEnded()
{
	if ( ConstructionPanel )
	{
		ConstructionPanel->SetPanelVisible( false );
	}
}

void UGameHUDWidget::HandleBonusPreviewUpdated( const TArray<FBonusIconData>& BonusIcons )
{
	ClearBonusIcons();
	CachedBonusData_ = BonusIcons;

	if ( !BonusIconCanvas || !BonusIconWidgetClass )
	{
		return;
	}

	APlayerController* pc = GetOwningPlayer();
	if ( !pc )
	{
		return;
	}

	TMap<FString, TArray<FBonusIconData>> grouped;
	for ( const FBonusIconData& bonus : BonusIcons )
	{
		FString key = bonus.WorldLocation.ToString();
		grouped.FindOrAdd( key ).Add( bonus );
	}

	for ( auto& kvp : grouped )
	{
		const TArray<FBonusIconData>& bonuses = kvp.Value;
		if ( bonuses.IsEmpty() )
		{
			continue;
		}

		UBonusIconWidget* iconWidget = CreateWidget<UBonusIconWidget>( pc, BonusIconWidgetClass );
		if ( !iconWidget )
		{
			continue;
		}

		for ( const FBonusIconData& bonus : bonuses )
		{
			iconWidget->AddEntry( bonus );
		}
		iconWidget->SetBuildingIcon( bonuses[0].BuildingIcon );
		UCanvasPanelSlot* slot = BonusIconCanvas->AddChildToCanvas( iconWidget );

		if ( slot )
		{
			slot->SetAutoSize( true );
			slot->SetAlignment( FVector2D( 0.5f, 1.0f ) );
		}

		ActiveBonusIcons_.Add( iconWidget );
		ActiveBonusWorldPositions_.Add( bonuses[0].WorldLocation );
	}
	UpdateBonusIconPositions();
}

void UGameHUDWidget::NativeTick( const FGeometry& MyGeometry, float InDeltaTime )
{
	Super::NativeTick( MyGeometry, InDeltaTime );

	if ( ActiveBonusIcons_.Num() > 0 )
	{
		UpdateBonusIconPositions();
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			if ( gL->GetCurrentPhase() == EGameLoopPhase::Combat )
			{
				UpdateWaveInfo();
			}
		}
	}
}

void UGameHUDWidget::UpdateBonusIconPositions()
{
	APlayerController* pc = GetOwningPlayer();

	if ( !pc )
	{
		return;
	}

	const float viewportScale = UWidgetLayoutLibrary::GetViewportScale( this );
	if ( viewportScale <= 0.0f )
	{
		return;
	}

	const float zoomAlpha = CameraZoomUtils::GetCameraZoomAlpha( this );

	const float buildingHeight = 80.0f;

	const float worldPadding = -15.0f;

	for ( int32 i = 0; i < ActiveBonusIcons_.Num(); ++i )
	{
		if ( !IsValid( ActiveBonusIcons_[i] ) )
		{
			continue;
		}
		if ( !ActiveBonusWorldPositions_.IsValidIndex( i ) )
		{
			continue;
		}

		FVector topWorldPos = ActiveBonusWorldPositions_[i] + FVector( 0.0f, 0.0f, buildingHeight + worldPadding );

		FVector2D screenPos;
		if ( !pc->ProjectWorldLocationToScreen( topWorldPos, screenPos ) )
		{
			ActiveBonusIcons_[i]->SetVisibility( ESlateVisibility::Collapsed );
			continue;
		}

		screenPos /= viewportScale;

		ActiveBonusIcons_[i]->SetRenderTransformPivot( FVector2D( 0.5f, 1.0f ) );

		UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>( ActiveBonusIcons_[i]->Slot.Get() );
		if ( !slot )
		{
			continue;
		}

		slot->SetAlignment( FVector2D( 0.5f, 1.0f ) );
		slot->SetPosition( screenPos );

		ActiveBonusIcons_[i]->ApplyCameraScale( zoomAlpha );
		ActiveBonusIcons_[i]->SetVisibility( ESlateVisibility::HitTestInvisible );
	}
}

void UGameHUDWidget::ClearBonusIcons()
{
	for ( UBonusIconWidget* icon : ActiveBonusIcons_ )
	{
		if ( icon )
		{
			icon->RemoveFromParent();
		}
	}
	ActiveBonusIcons_.Empty();
	ActiveBonusWorldPositions_.Empty();
}

void UGameHUDWidget::UpdateStatusText()
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

	EGameLoopPhase phase = gL->GetCurrentPhase();

	if ( Strokestatus )
	{
		if ( phase == EGameLoopPhase::Building )
		{
			int32 turn = gL->GetCurrentBuildTurn();
			UTexture2D* textureToUse = ( turn == 1 ) ? BackMorningTexture : BackEveningTexture;
			if ( textureToUse )
			{
				Strokestatus->SetBrushFromTexture( textureToUse );
			}
		}
	}
}

void UGameHUDWidget::UpdateButtonVisibility()
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

	EGameLoopPhase phase = gL->GetCurrentPhase();

	if ( ButtonEndTurn )
	{
		bool bShow = ( phase == EGameLoopPhase::Building );
		ButtonEndTurn->SetVisibility( bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}
}

void UGameHUDWidget::OnEndTurnClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->EndBuildTurn();
		}
	}

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_ENDTURN_CLICKED } );
}

void UGameHUDWidget::OnRelocateBuildingClicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Relocate building clicked" ) );

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_MOVEBUILDING_CLICKED } );

	UCoreManager* coreManager = UCoreManager::Get( this );
	if ( !coreManager )
	{
		return;
	}

	if ( UGameLoopManager* gL = coreManager->GetGameLoop() )
	{
		if ( gL->GetCurrentPhase() == EGameLoopPhase::Combat )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Relocate: blocked during combat phase" ) );
			return;
		}
	}

	USelectionManagerComponent* selectionManager = coreManager->GetSelectionManager();
	ABuildManager* buildManager = coreManager->GetBuildManager();
	UResourceManager* resourceManager = coreManager->GetResourceManager();

	if ( !selectionManager || !buildManager || !resourceManager )
	{
		return;
	}

	ABuilding* selectedBuilding = selectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding || !selectedBuilding->CanBeRelocated() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Relocate: building invalid" ) );
		return;
	}

	const int32 cost = selectedBuilding->GetRelocationGoldCost();

	if ( !resourceManager->HasEnoughResource( EResourceType::Gold, cost ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Relocate: not enough gold" ) );
		return;
	}

	buildManager->StartRelocatingBuilding( selectedBuilding );

	selectionManager->ClearSelection();
	HandleSelectionChanged();
	UpdateExtraButtonsVisibility();
}

void UGameHUDWidget::OnRemoveBuildingClicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Remove building clicked" ) );

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_DEMOLISHBUILDING_CLICKED } );

	UCoreManager* coreManager = UCoreManager::Get( this );
	if ( !coreManager )
	{
		return;
	}

	if ( UGameLoopManager* gL = coreManager->GetGameLoop() )
	{
		if ( gL->GetCurrentPhase() == EGameLoopPhase::Combat )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Remove: blocked during combat phase" ) );
			return;
		}
	}

	USelectionManagerComponent* selectionManager = coreManager->GetSelectionManager();
	ABuildManager* buildManager = coreManager->GetBuildManager();
	UResourceManager* resourceManager = coreManager->GetResourceManager();

	if ( !selectionManager || !buildManager || !resourceManager )
	{
		return;
	}

	ABuilding* selectedBuilding = selectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding || !selectedBuilding->CanBeRemoved() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Remove: building invalid" ) );
		return;
	}

	const FResourceProduction refund = selectedBuilding->GetDemolitionRefund();

	if ( !buildManager->RemoveExistingBuilding( selectedBuilding ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Remove: demolition failed, no refund granted" ) );
		return;
	}

	resourceManager->AddResources( refund );

	selectionManager->ClearSelection();
	HandleSelectionChanged();
	UpdateExtraButtonsVisibility();
}
void UGameHUDWidget::UpdateWaveInfo()
{
	if ( !IsValid( ActiveWavePanel ) )
	{
		UE_LOG( LogTemp, Warning,
		        TEXT( "GameHUD::UpdateWaveInfo skipped — ActiveWavePanel null (WaveInfoPanel bound=%s, WavePanelClass=%s)" ),
		        IsValid( WaveInfoPanel ) ? TEXT( "yes" ) : TEXT( "no" ),
		        IsValid( WavePanelClass ) ? TEXT( "yes" ) : TEXT( "no" ) );
		return;
	}

	UCoreManager* core = UCoreManager::Get( this );
	UGameLoopManager* gameLoop = IsValid( core ) ? core->GetGameLoop() : nullptr;
	AWaveManager* waveManager = IsValid( core ) ? core->GetWaveManager() : nullptr;

	if ( IsValid( gameLoop ) && IsValid( waveManager ) )
	{
		int32 waveIndex = FMath::Max( 0, gameLoop->GetCurrentWave() - 1 );

		TMap<TSubclassOf<AUnit>, int32> waveData;

		if ( gameLoop->GetCurrentPhase() == EGameLoopPhase::Combat )
		{
			waveData = waveManager->GetCurrentWaveRemainingEnemies();

			if ( waveData.Num() == 0 )
			{
				waveData = waveManager->GetNextWaveComposition( waveIndex );
			}
		}
		else
		{
			waveData = waveManager->GetNextWaveComposition( waveIndex );
		}

		ActiveWavePanel->PopulatePanel( waveData );
	}
}

void UGameHUDWidget::HandleGameEnded( EGameResult Result )
{
	if ( ActiveOverlay )
	{
		ActiveOverlay->RemoveFromParent();
		ActiveOverlay = nullptr;
	}

	// Endless и Abandoned не имеют классических баннеров победы/поражения;
	// финал бесконечного режима показывается MatchResultsWidget'ом.
	if ( Result == EGameResult::EndlessRun || Result == EGameResult::Abandoned )
	{
		return;
	}

	const bool bVictory = ( Result == EGameResult::Win );
	TSubclassOf<UGameStateOverlayWidget> ClassToUse = bVictory ? WinWidgetClass : LoseWidgetClass;
	if ( !ClassToUse )
	{
		return;
	}

	ActiveOverlay = CreateWidget<UGameStateOverlayWidget>( this, ClassToUse );
	if ( ActiveOverlay )
	{
		ActiveOverlay->AddToViewport( 100 );
	}

	if ( APlayerController* PC = GetOwningPlayer() )
	{
		if ( AStrategyCamera* Cam = Cast<AStrategyCamera>( PC->GetPawn() ) )
		{
			Cam->SetCameraInputDisabled( true );
		}
	}
}

void UGameHUDWidget::TogglePauseMenu()
{
	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		if ( !session->IsGameStarted() )
		{
			return;
		}
	}

	if ( ActiveOverlay )
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_RESUMEGAME } );

		ActiveOverlay->OnResumeRequested.RemoveDynamic( this, &UGameHUDWidget::TogglePauseMenu );

		ActiveOverlay->RemoveFromParent();
		ActiveOverlay = nullptr;

		if ( APlayerController* PC = GetOwningPlayer() )
		{
			if ( AStrategyCamera* Cam = Cast<AStrategyCamera>( PC->GetPawn() ) )
			{
				Cam->SetCameraInputDisabled( false );
			}
		}
	}
	else
	{
		if ( !PauseWidgetClass )
		{
			return;
		}

		OnAudioEvent_.Broadcast( { AudioTags::SFX_PAUSEGAME } );

		ActiveOverlay = CreateWidget<UGameStateOverlayWidget>( this, PauseWidgetClass );
		if ( ActiveOverlay )
		{
			ActiveOverlay->AddToViewport( 100 );
			ActiveOverlay->OnResumeRequested.AddDynamic( this, &UGameHUDWidget::TogglePauseMenu );
		}

		if ( APlayerController* PC = GetOwningPlayer() )
		{
			if ( AStrategyCamera* Cam = Cast<AStrategyCamera>( PC->GetPawn() ) )
			{
				Cam->SetCameraInputDisabled( true );
			}
		}
	}
}

bool UGameHUDWidget::AddBossBar( UHealthBarWidget* bar )
{
	if ( !bar )
	{
		return false;
	}
	if ( !BossBarsContainer )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UGameHUDWidget::AddBossBar: BossBarsContainer is not bound in WBP_GameHUD" ) );
		return false;
	}
	BossBarsContainer->AddChildToHorizontalBox( bar );
	return true;
}

void UGameHUDWidget::RemoveBossBar( UHealthBarWidget* bar )
{
	if ( !bar || !BossBarsContainer )
	{
		return;
	}
	BossBarsContainer->RemoveChild( bar );
}

void UGameHUDWidget::HandleSelectionChanged()
{
	UE_LOG( LogTemp, Warning, TEXT( "UGameHUDWidget::HandleSelectionChanged" ) );

	UpdateExtraButtonsVisibility();

	UCoreManager* coreManager = UCoreManager::Get( this );
	if ( !coreManager )
		return;

	USelectionManagerComponent* selectionManager = coreManager->GetSelectionManager();
	if ( !selectionManager )
		return;

	AActor* selectedActor = selectionManager->GetPrimarySelectedActor();
	ABuilding* selectedBuilding = Cast<ABuilding>( selectedActor );
	AUnit* selectedUnit = Cast<AUnit>( selectedActor );

	ABuildManager* buildManager = nullptr;
	if ( APlayerController* pc = GetOwningPlayer() )
	{
		if ( auto* debugPC = Cast<ADebugPlayerController>( pc ) )
		{
			buildManager = debugPC->GetBuildManager();
		}
	}

	if ( buildManager )
	{
		buildManager->HideAllDefensiveRanges();
	}

	if ( IsValid( selectedBuilding ) )
	{
		HideTooltipForEnemy();
		if ( BuildingPanel )
		{
			BuildingPanel->ShowTooltipForBuilding( selectedBuilding );
		}

		if ( ADefensiveBuilding* defensive = Cast<ADefensiveBuilding>( selectedBuilding ) )
		{
			defensive->ShowAttackRange();
		}
	}
	else if ( IsValid( selectedUnit ) )
	{
		if ( BuildingPanel )
		{
			BuildingPanel->HideTooltipForBuilding();
		}

		if ( IsValid( ActiveEnemyTooltip ) )
		{
			ActiveEnemyTooltip->ShowTooltip( selectedUnit );
		}
	}
	else
	{
		if ( BuildingPanel )
		{
			BuildingPanel->HideTooltipForBuilding();
		}
		HideTooltipForEnemy();
	}
}
void UGameHUDWidget::UpdateExtraButtonsVisibility()
{
	UCoreManager* coreManager = UCoreManager::Get( this );
	if ( !coreManager )
	{
		if ( ButtonRelocateBuilding )
		{
			ButtonRelocateBuilding->SetVisibility( ESlateVisibility::Collapsed );
		}

		if ( ButtonRemoveBuilding )
		{
			ButtonRemoveBuilding->SetVisibility( ESlateVisibility::Collapsed );
		}

		return;
	}

	USelectionManagerComponent* selectionManager = coreManager->GetSelectionManager();
	ABuildManager* buildManager = coreManager->GetBuildManager();
	UGameLoopManager* gL = coreManager->GetGameLoop();

	ABuilding* selectedBuilding = selectionManager ? selectionManager->GetPrimarySelectedBuilding() : nullptr;
	const bool bHasSelectedBuilding = IsValid( selectedBuilding );

	const bool bIsPlacing = buildManager && buildManager->IsPlacing();

	const bool bIsCombatPhase = gL && gL->GetCurrentPhase() == EGameLoopPhase::Combat;

	const bool bShowRelocateButton = bHasSelectedBuilding && !bIsPlacing && selectedBuilding->CanBeRelocated();

	const bool bShowRemoveButton = bHasSelectedBuilding && !bIsPlacing && selectedBuilding->CanBeRemoved();

	if ( ButtonRelocateBuilding )
	{
		ButtonRelocateBuilding->SetVisibility(
		    bShowRelocateButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
		ButtonRelocateBuilding->SetIsEnabled( bShowRelocateButton && !bIsCombatPhase );
		ButtonRelocateBuilding->SetRenderOpacity( bIsCombatPhase ? 0.4f : 1.0f );
	}

	if ( ButtonRemoveBuilding )
	{
		ButtonRemoveBuilding->SetVisibility(
		    bShowRemoveButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
		ButtonRemoveBuilding->SetIsEnabled( bShowRemoveButton && !bIsCombatPhase );
		ButtonRemoveBuilding->SetRenderOpacity( bIsCombatPhase ? 0.4f : 1.0f );
	}
}

void UGameHUDWidget::InitSelectionManager( USelectionManagerComponent* InSelectionManager )
{
	UE_LOG( LogTemp, Warning, TEXT( "InitSelectionManager called: %s" ), *GetNameSafe( InSelectionManager ) );

	if ( SelectionManager == InSelectionManager )
	{
		UE_LOG( LogTemp, Warning, TEXT( "InitSelectionManager: same manager, skipping rebind" ) );
		return;
	}

	if ( SelectionManager )
	{
		SelectionManager->OnSelectionChanged.RemoveDynamic( this, &UGameHUDWidget::HandleSelectionChanged );
	}

	SelectionManager = InSelectionManager;

	if ( !SelectionManager )
	{
		UE_LOG( LogTemp, Error, TEXT( "InitSelectionManager: SelectionManager is null" ) );
		if ( BuildingPanel )
		{
			BuildingPanel->HideTooltipForBuilding();
		}
		return;
	}

	SelectionManager->OnSelectionChanged.AddUniqueDynamic( this, &UGameHUDWidget::HandleSelectionChanged );

	UE_LOG( LogTemp, Warning, TEXT( "InitSelectionManager: subscribed to OnSelectionChanged" ) );

	HandleSelectionChanged();
}

void UGameHUDWidget::HideTooltipForBuilding()
{
	if ( BuildingPanel )
	{
		BuildingPanel->HideTooltipForBuilding();
	}
}

void UGameHUDWidget::HideTooltipForEnemy()
{
	if ( IsValid( ActiveEnemyTooltip ) )
	{
		ActiveEnemyTooltip->HideTooltip();
	}
}