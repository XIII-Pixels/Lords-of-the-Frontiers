#include "Lords_Frontiers/Public/UI/GameHUD.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Building/DefensiveBuilding.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"
#include "UI/HealthBar/HealthBarWidget.h"
#include "UI/Widgets/BuildingTooltipWidget.h"
#include "UI/Widgets/GameStateOverlayWidget.h"
#include "UI/Widgets/StageProgressWidget.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraZoomUtils.h"
#include "Camera/StrategyCamera.h"
#include "Components/GridPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UResourceManager* rM = core->GetResourceManager() )
		{
			rM->OnResourceChanged.AddUniqueDynamic( this, &UGameHUDWidget::HandleResourceChanged );
		}

		if ( UEconomyComponent* eC = core->GetEconomyComponent() )
		{
			eC->OnNetIncomeChanged.AddUniqueDynamic( this, &UGameHUDWidget::HandleNetIncomeChanged );
		}
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
	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->OnClicked.AddDynamic( this, &UGameHUDWidget::OnDefensiveBuildingsClicked );
		ButtonDefensiveBuildings->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverDefensiveBuildings );
	}
	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->OnClicked.AddDynamic( this, &UGameHUDWidget::OnEconomyBuildingClicked );
		ButtonEconomyBuilding->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverEconomyBuilding );
	}
	if ( ButtonEndTurn )
	{
		ButtonEndTurn->OnClicked.AddDynamic( this, &UGameHUDWidget::OnEndTurnClicked );
		ButtonEndTurn->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverEndTurn );
	}

	if ( ButtonBuildingWoodenHouse )
	{
		ButtonBuildingWoodenHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildWoodenHouseClicked );
		ButtonBuildingWoodenHouse->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverWoodenHouse );
		ButtonBuildingWoodenHouse->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStrawHouse )
	{
		ButtonBuildingStrawHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildStrawHouseClicked );
		ButtonBuildingStrawHouse->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverStrawHouse );
		ButtonBuildingStrawHouse->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingFarm )
	{
		ButtonBuildingFarm->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildFarmClicked );
		ButtonBuildingFarm->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverFarm );
		ButtonBuildingFarm->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingLawnHouse )
	{
		ButtonBuildingLawnHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildLawnHouseClicked );
		ButtonBuildingLawnHouse->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverLawnHouse );
		ButtonBuildingLawnHouse->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMagicHouse )
	{
		ButtonBuildingMagicHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildMagicHouseClicked );
		ButtonBuildingMagicHouse->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverMagicHouse );
		ButtonBuildingMagicHouse->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}

	if ( ButtonBuildingWoodWall )
	{
		ButtonBuildingWoodWall->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildWoodWallClicked );
		ButtonBuildingWoodWall->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverWoodWall );
		ButtonBuildingWoodWall->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStoneWall )
	{
		ButtonBuildingStoneWall->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildStoneWallClicked );
		ButtonBuildingStoneWall->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverStoneWall );
		ButtonBuildingStoneWall->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT0 )
	{
		ButtonBuildingTowerT0->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerT0Clicked );
		ButtonBuildingTowerT0->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverTowerT0 );
		ButtonBuildingTowerT0->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT1 )
	{
		ButtonBuildingTowerT1->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerT1Clicked );
		ButtonBuildingTowerT1->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverTowerT1 );
		ButtonBuildingTowerT1->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT2 )
	{
		ButtonBuildingTowerT2->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerT2Clicked );
		ButtonBuildingTowerT2->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverTowerT2 );
		ButtonBuildingTowerT2->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}

	if ( ButtonBuildingMortira )
	{
		ButtonBuildingMortira->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerMortiraClicked );
		ButtonBuildingMortira->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverTowerMortira );
		ButtonBuildingMortira->OnUnhovered.AddDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}

	ABuildManager* buildManager =
	    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) );
	if ( buildManager )
	{
		buildManager->OnBonusPreviewUpdated.AddDynamic( this, &UGameHUDWidget::HandleBonusPreviewUpdated );
		buildManager->OnPlacingCancelled.AddDynamic( this, &UGameHUDWidget::OnPlacingCancelled );
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

	if ( BtnToggleWaveInfo )
	{
		BtnToggleWaveInfo->OnClicked.AddDynamic( this, &UGameHUDWidget::OnWaveInfoButtonClicked );
		BtnToggleWaveInfo->OnHovered.AddDynamic( this, &UGameHUDWidget::OnHoverWaveInfoButton );
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

	InitializeTooltipWidget( EconomyTooltipClass, ActiveEconomyTooltip );
	InitializeTooltipWidget( DefensiveTooltipClass, ActiveDefensiveTooltip );

	InitIncomeDisplay();

	InitIncomeDisplay();

	UpdateDayText();
	UpdateStatusText();
	UpdateResources();
	UpdateButtonVisibility();
	UpdateBuildingUIVisibility();

	ShowEconomyBuildings();

	UpdateAllBuildingButtons();
	if ( IsValid( WavePanelClass ) && !IsValid( ActiveWavePanel ) )
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
	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnDefensiveBuildingsClicked );
		ButtonDefensiveBuildings->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverDefensiveBuildings );
	}
	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnEconomyBuildingClicked );
		ButtonEconomyBuilding->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverEconomyBuilding );
	}

	if ( ButtonEndTurn )
	{
		ButtonEndTurn->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnEndTurnClicked );
		ButtonEndTurn->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverEndTurn );
	}

	if ( ButtonBuildingWoodenHouse )
	{
		ButtonBuildingWoodenHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildWoodenHouseClicked );
		ButtonBuildingWoodenHouse->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverWoodenHouse );
		ButtonBuildingWoodenHouse->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStrawHouse )
	{
		ButtonBuildingStrawHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildStrawHouseClicked );
		ButtonBuildingStrawHouse->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverStrawHouse );
		ButtonBuildingStrawHouse->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingFarm )
	{
		ButtonBuildingFarm->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildFarmClicked );
		ButtonBuildingFarm->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverFarm );
		ButtonBuildingFarm->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingLawnHouse )
	{
		ButtonBuildingLawnHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildLawnHouseClicked );
		ButtonBuildingLawnHouse->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverLawnHouse );
		ButtonBuildingLawnHouse->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMagicHouse )
	{
		ButtonBuildingMagicHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildMagicHouseClicked );
		ButtonBuildingMagicHouse->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverMagicHouse );
		ButtonBuildingMagicHouse->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}

	if ( ButtonBuildingWoodWall )
	{
		ButtonBuildingWoodWall->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildWoodWallClicked );
		ButtonBuildingWoodWall->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverWoodWall );
		ButtonBuildingWoodWall->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStoneWall )
	{
		ButtonBuildingStoneWall->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildStoneWallClicked );
		ButtonBuildingStoneWall->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverStoneWall );
		ButtonBuildingStoneWall->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT0 )
	{
		ButtonBuildingTowerT0->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT0Clicked );
		ButtonBuildingTowerT0->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverTowerT0 );
		ButtonBuildingTowerT0->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT1 )
	{
		ButtonBuildingTowerT1->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT1Clicked );
		ButtonBuildingTowerT1->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverTowerT1 );
		ButtonBuildingTowerT1->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT2 )
	{
		ButtonBuildingTowerT2->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT2Clicked );
		ButtonBuildingTowerT2->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverTowerT2 );
		ButtonBuildingTowerT2->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMortira )
	{
		ButtonBuildingMortira->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerMortiraClicked );
		ButtonBuildingMortira->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverTowerMortira );
		ButtonBuildingMortira->OnUnhovered.RemoveDynamic( this, &UGameHUDWidget::OnBuildingUnhovered );
	}

	if ( BtnToggleWaveInfo )
	{
		BtnToggleWaveInfo->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnWaveInfoButtonClicked );
		BtnToggleWaveInfo->OnHovered.RemoveDynamic( this, &UGameHUDWidget::OnHoverWaveInfoButton );
	}

	ABuildManager* buildManager =
	    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) );
	if ( buildManager )
	{
		buildManager->OnBonusPreviewUpdated.RemoveDynamic( this, &UGameHUDWidget::HandleBonusPreviewUpdated );
		buildManager->OnPlacingCancelled.RemoveDynamic( this, &UGameHUDWidget::OnPlacingCancelled );
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

		if ( UResourceManager* rM = core->GetResourceManager() )
		{
			rM->OnResourceChanged.RemoveDynamic( this, &UGameHUDWidget::HandleResourceChanged );
		}

		if ( UEconomyComponent* eC = core->GetEconomyComponent() )
		{
			eC->OnNetIncomeChanged.RemoveDynamic( this, &UGameHUDWidget::HandleNetIncomeChanged );
		}
	}

	// Sound
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

	if ( StageProgressBar && MaxTurns > 0 )
	{
		float progress = static_cast<float>( CurrentTurn - 1 ) / static_cast<float>( MaxTurns );
		StageProgressBar->SetTargetProgress( FMath::Clamp( progress, 0.0f, 1.0f ) );
	}
}

void UGameHUDWidget::HandleResourceChanged( EResourceType Type, int32 NewAmount )
{
	UpdateResources();
	UpdateAllBuildingButtons();

	if ( ActiveEconomyTooltip && ActiveEconomyTooltip->GetVisibility() != ESlateVisibility::Hidden )
	{
		ActiveEconomyTooltip->UpdateContent();
	}
	if ( ActiveDefensiveTooltip && ActiveDefensiveTooltip->GetVisibility() != ESlateVisibility::Hidden )
	{
		ActiveDefensiveTooltip->UpdateContent();
	}
	if ( bIsBuildingLocked && LockedBuildingClass )
	{
		UCoreManager* core = UCoreManager::Get( this );
		UResourceManager* rM = core ? core->GetResourceManager() : nullptr;
		if ( rM )
		{
			const ABuilding* buildingCDO = LockedBuildingClass->GetDefaultObject<ABuilding>();
			if ( buildingCDO && !rM->CanAfford( buildingCDO->GetBuildingCost() ) )
			{
				if ( ABuildManager* bM = core->GetBuildManager() )
				{
					bM->CancelPlacing();
				}

				bIsBuildingLocked = false;
				LockedBuildingClass = nullptr;

				if ( ActiveEconomyTooltip )
				{
					ActiveEconomyTooltip->SetLocked( false );
					ActiveEconomyTooltip->StartAutoHideTimer();
				}
				if ( ActiveDefensiveTooltip )
				{
					ActiveDefensiveTooltip->SetLocked( false );
					ActiveDefensiveTooltip->StartAutoHideTimer();
				}
			}
		}
	}
}

void UGameHUDWidget::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	UpdateDayText();
	UpdateStatusText();
	UpdateButtonVisibility();
	UpdateBuildingUIVisibility();

	if ( NewPhase == EGameLoopPhase::Combat )
	{
		CancelCurrentBuilding();
	}

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

	if ( !bIsEconomySubscribed_ )
	{
		if ( UCoreManager* core = UCoreManager::Get( this ) )
		{
			UResourceManager* rM = core->GetResourceManager();
			UEconomyComponent* eC = core->GetEconomyComponent();

			if ( rM && eC )
			{
				rM->OnResourceChanged.AddUniqueDynamic( this, &UGameHUDWidget::HandleResourceChanged );
				eC->OnNetIncomeChanged.AddUniqueDynamic( this, &UGameHUDWidget::HandleNetIncomeChanged );
				UpdateResources();
				InitIncomeDisplay();
				bIsEconomySubscribed_ = true;
			}
		}
	}

	if ( ActiveBonusIcons_.Num() > 0 )
	{
		UpdateBonusIconPositions();
	}

	TickIncomeAnimation( Text_GoldIncome, Arrow_Gold, GoldIncomeAnim_, InDeltaTime );
	TickIncomeAnimation( Text_FoodIncome, Arrow_Food, FoodIncomeAnim_, InDeltaTime );

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

void UGameHUDWidget::UpdateDayText()
{
	if ( !TextDay )
	{
		return;
	}

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

	int32 wave = gL->GetCurrentWave();
	TextDay->SetText( FText::FromString( FString::Printf( TEXT( "День %d" ), wave ) ) );
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

void UGameHUDWidget::UpdateResources()
{
	UE_LOG( LogTemp, Warning, TEXT( "=== UpdateResources ===" ) );

	UCoreManager* core = UCoreManager::Get( this );
	UResourceManager* rM = core ? core->GetResourceManager() : nullptr;

	if ( !rM )
	{
		return;
	}

	int32 gold = rM->GetResourceAmount( EResourceType::Gold );
	int32 food = rM->GetResourceAmount( EResourceType::Food );
	int32 pop = rM->GetResourceAmount( EResourceType::Population );

	UE_LOG( LogTemp, Warning, TEXT( "Resources: Gold=%d, Food=%d, Pop=%d" ), gold, food, pop );

	if ( Text_Gold )
	{
		Text_Gold->SetText( FText::AsNumber( gold ) );
	}

	if ( Text_Food )
	{
		Text_Food->SetText( FText::AsNumber( food ) );
	}

	if ( Text_Citizens )
	{
		Text_Citizens->SetText( FText::AsNumber( pop ) );
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
void UGameHUDWidget::OnDefensiveBuildingsClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDINGCATEGORY_CLICKED } );

	ShowDefensiveBuildings();
}

void UGameHUDWidget::OnEconomyBuildingClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDINGCATEGORY_CLICKED } );

	ShowEconomyBuildings();
}

void UGameHUDWidget::ShowEconomyBuildings()
{
	bShowingEconomyBuildings_ = true;

	if ( EconomyCardBox )
	{
		EconomyCardBox->SetVisibility( ESlateVisibility::Visible );
	}

	if ( DefensiveCardBox )
	{
		DefensiveCardBox->SetVisibility( ESlateVisibility::Collapsed );
	}

	UpdateCategoryButtonsVisual();
}

void UGameHUDWidget::ShowDefensiveBuildings()
{
	bShowingEconomyBuildings_ = false;

	if ( EconomyCardBox )
	{
		EconomyCardBox->SetVisibility( ESlateVisibility::Collapsed );
	}

	if ( DefensiveCardBox )
	{
		DefensiveCardBox->SetVisibility( ESlateVisibility::Visible );
	}

	UpdateCategoryButtonsVisual();
}

void UGameHUDWidget::StartBuilding( TSubclassOf<ABuilding> BuildingClass )
{
	if ( !BuildingClass )
	{
		return;
	}

	UCoreManager* core = UCoreManager::Get( this );
	ABuildManager* bM = core ? core->GetBuildManager() : nullptr;
	if ( !bM )
	{
		return;
	}

	if ( UResourceManager* rM = core->GetResourceManager() )
	{
		const ABuilding* buildingCDO = BuildingClass->GetDefaultObject<ABuilding>();
		if ( buildingCDO && !rM->CanAfford( buildingCDO->GetBuildingCost() ) )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Red, TEXT( "Not enough resources!" ) );
			}
			return;
		}
	}

	bM->StartPlacingBuilding( BuildingClass );

	bIsBuildingLocked = true;
	LockedBuildingClass = BuildingClass;
}

void UGameHUDWidget::OnBuildWoodenHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingWoodenHouse );

	StartBuilding( WoodenHouseClass );
}

void UGameHUDWidget::OnBuildStrawHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingStrawHouse );

	StartBuilding( StrawHouseClass );
}

void UGameHUDWidget::OnBuildFarmClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingFarm );

	StartBuilding( FarmClass );
}

void UGameHUDWidget::OnBuildLawnHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingLawnHouse );

	StartBuilding( LawnHouseClass );
}

void UGameHUDWidget::OnBuildMagicHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingMagicHouse );

	StartBuilding( MagicHouseClass );
}

void UGameHUDWidget::OnBuildWoodWallClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingWoodWall );

	StartBuilding( WoodWallClass );
}

void UGameHUDWidget::OnBuildStoneWallClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingStoneWall );

	StartBuilding( StoneWallClass );
}

void UGameHUDWidget::OnBuildTowerT0Clicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingTowerT0 );

	StartBuilding( TowerT0Class );
}

void UGameHUDWidget::OnBuildTowerT1Clicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingTowerT1 );

	StartBuilding( TowerT1Class );
}

void UGameHUDWidget::OnBuildTowerT2Clicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingTowerT2 );

	StartBuilding( TowerT2Class );
}

void UGameHUDWidget::OnBuildTowerMortiraClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingMortira );

	StartBuilding( TowerMortiraClass );
}

void UGameHUDWidget::UpdateBuildingUIVisibility()
{
	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return;
	};

	UGameLoopManager* gL = core->GetGameLoop();
	if ( !gL )
	{
		return;
	};

	EGameLoopPhase phase = gL->GetCurrentPhase();
	bool bShowBuildingUI = ( phase == EGameLoopPhase::Building );

	ESlateVisibility newVisibility = bShowBuildingUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if ( BackForButton )
	{
		BackForButton->SetVisibility( newVisibility );
	}

	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->SetVisibility( newVisibility );
	}
	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->SetVisibility( newVisibility );
	}

	if ( ButtonRelocateBuilding )
	{
		ButtonRelocateBuilding->SetVisibility( newVisibility );
	}
	if ( ButtonRemoveBuilding )
	{
		ButtonRemoveBuilding->SetVisibility( newVisibility );
	}

	if ( bShowBuildingUI )
	{
		if ( bShowingEconomyBuildings_ )
		{
			ShowEconomyBuildings();
		}
		else
		{
			ShowDefensiveBuildings();
		}
	}
	else
	{
		if ( EconomyCardBox )
		{
			EconomyCardBox->SetVisibility( ESlateVisibility::Collapsed );
		}
		if ( DefensiveCardBox )
		{
			DefensiveCardBox->SetVisibility( ESlateVisibility::Collapsed );
		}
	}
}

void UGameHUDWidget::CancelCurrentBuilding()
{
	UCoreManager* core = UCoreManager::Get( this );
	ABuildManager* bM = core ? core->GetBuildManager() : nullptr;

	if ( bM && bM->IsPlacing() )
	{
		bM->CancelPlacing();
	}

	bIsBuildingLocked = false;
	LockedBuildingClass = nullptr;

	if ( ActiveEconomyTooltip )
	{
		ActiveEconomyTooltip->SetLocked( false );
	}
	if ( ActiveDefensiveTooltip )
	{
		ActiveDefensiveTooltip->SetLocked( false );
	}
}

void UGameHUDWidget::OnPlacingCancelled()
{
	bIsBuildingLocked = false;
	LockedBuildingClass = nullptr;

	if ( ActiveEconomyTooltip )
	{
		ActiveEconomyTooltip->SetLocked( false );
		ActiveEconomyTooltip->HideTooltip();
	}
	if ( ActiveDefensiveTooltip )
	{
		ActiveDefensiveTooltip->SetLocked( false );
		ActiveDefensiveTooltip->HideTooltip();
	}
}

void UGameHUDWidget::UpdateCategoryButtonsVisual()
{
	if ( ButtonEconomyBuilding )
	{
		float OffsetY = bShowingEconomyBuildings_ ? ActiveButtonLiftOffset : 0.0f;
		ButtonEconomyBuilding->SetRenderTranslation( FVector2D( 0.0f, OffsetY ) );
	}

	if ( ButtonDefensiveBuildings )
	{
		float OffsetY = bShowingEconomyBuildings_ ? 0.0f : ActiveButtonLiftOffset;
		ButtonDefensiveBuildings->SetRenderTranslation( FVector2D( 0.0f, OffsetY ) );
	}
}

void UGameHUDWidget::UpdateAllBuildingButtons()
{
	UpdateButtonAvailability( ButtonBuildingWoodenHouse, WoodenHouseClass );
	UpdateButtonAvailability( ButtonBuildingStrawHouse, StrawHouseClass );
	UpdateButtonAvailability( ButtonBuildingFarm, FarmClass );
	UpdateButtonAvailability( ButtonBuildingLawnHouse, LawnHouseClass );
	UpdateButtonAvailability( ButtonBuildingMagicHouse, MagicHouseClass );

	UpdateButtonAvailability( ButtonBuildingWoodWall, WoodWallClass );
	UpdateButtonAvailability( ButtonBuildingStoneWall, StoneWallClass );
	UpdateButtonAvailability( ButtonBuildingTowerT0, TowerT0Class );
	UpdateButtonAvailability( ButtonBuildingTowerT1, TowerT1Class );
	UpdateButtonAvailability( ButtonBuildingTowerT2, TowerT2Class );
	UpdateButtonAvailability( ButtonBuildingMortira, TowerMortiraClass );
}

void UGameHUDWidget::UpdateButtonAvailability( UButton* button, TSubclassOf<ABuilding> buildingClass )
{
	if ( !button || !buildingClass )
	{
		return;
	}

	UCoreManager* core = UCoreManager::Get( this );
	UResourceManager* rM = core ? core->GetResourceManager() : nullptr;

	if ( !rM )
	{
		return;
	}

	const ABuilding* buildingCDO = buildingClass->GetDefaultObject<ABuilding>();
	bool bCanAfford = rM->CanAfford( buildingCDO->GetBuildingCost() );

	button->SetRenderOpacity( bCanAfford ? 1.0f : 0.4f );
	button->SetBackgroundColor( bCanAfford ? AffordableColor : TooExpensiveColor );
}

void UGameHUDWidget::OnBuildingUnhovered()
{
	if ( bIsBuildingLocked && LockedBuildingClass )
	{
		ShowTooltipForBuilding( LockedBuildingClass );
	}
	else
	{
		if ( ActiveEconomyTooltip )
		{
			ActiveEconomyTooltip->HideTooltip();
		}
		if ( ActiveDefensiveTooltip )
		{
			ActiveDefensiveTooltip->HideTooltip();
		}
	}
}

void UGameHUDWidget::PlayOnBuildingButtonClickedSound( const UButton* button ) const
{
	if ( button && button->GetBackgroundColor() == AffordableColor )
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDING_CLICKED } );
	}
	else
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDING_UNAFFORDABLE_CLICKED } );
	}
}

void UGameHUDWidget::PlayOnBuildingButtonHoveredSound( const UButton* button ) const
{
	if ( button && button->GetBackgroundColor() == AffordableColor )
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDING_HOVERED } );
	}
	else
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDING_UNAFFORDABLE_HOVERED } );
	}
}

void UGameHUDWidget::ToggleWaveInfoPanel()
{
	if ( IsValid( ActiveWavePanel ) )
	{
		ActiveWavePanel->TogglePanel();
	}
}

void UGameHUDWidget::UpdateWaveInfo()
{
	if ( !IsValid( ActiveWavePanel ) )
	{
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

		int32 totalEnemies = 0;
		for ( const TPair<TSubclassOf<AUnit>, int32>& pair : waveData )
		{
			totalEnemies += pair.Value;
		}

		if ( IsValid( Text_TotalEnemies ) )
		{
			Text_TotalEnemies->SetText( FText::AsNumber( totalEnemies ) );
		}

		ActiveWavePanel->PopulatePanel( waveData );
	}
}

void UGameHUDWidget::OnWaveInfoButtonClicked()
{
	ToggleWaveInfoPanel();
}

void UGameHUDWidget::InitIncomeDisplay()
{
	if ( Arrow_Gold )
	{
		Arrow_Gold->SetVisibility( ESlateVisibility::Collapsed );
	}
	if ( Arrow_Food )
	{
		Arrow_Food->SetVisibility( ESlateVisibility::Collapsed );
	}

	ApplyIncomeText( Text_GoldIncome, 0 );
	ApplyIncomeText( Text_FoodIncome, 0 );

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UEconomyComponent* eC = core->GetEconomyComponent() )
		{
			eC->OnNetIncomeChanged.RemoveDynamic( this, &UGameHUDWidget::HandleNetIncomeChanged );
			eC->OnNetIncomeChanged.AddDynamic( this, &UGameHUDWidget::HandleNetIncomeChanged );

			FResourceProduction netIncome = eC->CalculateNetIncome();
			GoldIncomeAnim_.DisplayedValue = netIncome.Gold;
			GoldIncomeAnim_.TargetValue = netIncome.Gold;
			FoodIncomeAnim_.DisplayedValue = netIncome.Food;
			FoodIncomeAnim_.TargetValue = netIncome.Food;

			ApplyIncomeText( Text_GoldIncome, netIncome.Gold );
			ApplyIncomeText( Text_FoodIncome, netIncome.Food );
		}
	}
}

void UGameHUDWidget::HandleNetIncomeChanged( const FResourceProduction& netIncome )
{
	StartIncomeAnimation( Text_GoldIncome, Arrow_Gold, GoldIncomeAnim_, netIncome.Gold );
	StartIncomeAnimation( Text_FoodIncome, Arrow_Food, FoodIncomeAnim_, netIncome.Food );
}

void UGameHUDWidget::StartIncomeAnimation(
    UTextBlock* textBlock, UImage* arrow, FIncomeAnimState& state, int32 newValue
)
{
	if ( newValue == state.TargetValue )
	{
		return;
	}

	state.StartValue = state.DisplayedValue;
	state.TargetValue = newValue;
	state.Elapsed = 0.0f;
	state.bAnimating = true;
	state.ArrowTimer = ArrowDisplayDuration;

	ApplyIncomeText( textBlock, state.DisplayedValue );

	if ( arrow )
	{
		bool bIncrease = ( newValue > state.StartValue );
		UTexture2D* arrowTexture = bIncrease ? ArrowUpTexture.Get() : ArrowDownTexture.Get();

		if ( arrowTexture )
		{
			arrow->SetBrushFromTexture( arrowTexture );
		}

		arrow->SetColorAndOpacity(
		    bIncrease ? PositiveIncomeColor.GetSpecifiedColor() : NegativeIncomeColor.GetSpecifiedColor()
		);
		arrow->SetRenderOpacity( 1.0f );
		arrow->SetVisibility( ESlateVisibility::HitTestInvisible );
	}
}

void UGameHUDWidget::TickIncomeAnimation(
    UTextBlock* textBlock, UImage* arrow, FIncomeAnimState& state, float deltaTime
)
{
	const float cArrowFadeDuration = 0.5f;

	if ( state.bAnimating )
	{
		state.Elapsed += deltaTime;
		float alpha = FMath::Clamp( state.Elapsed / FMath::Max( IncomeAnimationDuration, 0.01f ), 0.0f, 1.0f );

		state.DisplayedValue = FMath::RoundToInt(
		    FMath::Lerp( static_cast<float>( state.StartValue ), static_cast<float>( state.TargetValue ), alpha )
		);

		ApplyIncomeText( textBlock, state.DisplayedValue );

		if ( alpha >= 1.0f )
		{
			state.bAnimating = false;
			state.DisplayedValue = state.TargetValue;
			ApplyIncomeText( textBlock, state.DisplayedValue );
		}
	}

	if ( state.ArrowTimer > 0.0f )
	{
		state.ArrowTimer -= deltaTime;

		if ( state.ArrowTimer <= 0.0f && arrow )
		{
			arrow->SetVisibility( ESlateVisibility::Collapsed );
		}
		else if ( arrow && state.ArrowTimer < cArrowFadeDuration )
		{
			float fadeAlpha = FMath::Max( 0.0f, state.ArrowTimer / cArrowFadeDuration );
			arrow->SetRenderOpacity( fadeAlpha );
		}
	}
}

void UGameHUDWidget::ApplyIncomeText( UTextBlock* textBlock, int32 value )
{
	if ( !textBlock )
	{
		return;
	}

	FString displayText;
	if ( value > 0 )
	{
		displayText = FString::Printf( TEXT( "+%d" ), value );
		textBlock->SetColorAndOpacity( PositiveIncomeColor );
	}
	else if ( value < 0 )
	{
		displayText = FString::Printf( TEXT( "%d" ), value );
		textBlock->SetColorAndOpacity( NegativeIncomeColor );
	}
	else
	{
		displayText = TEXT( "0" );
		textBlock->SetColorAndOpacity( FSlateColor( FLinearColor::White ) );
	}

	textBlock->SetText( FText::FromString( displayText ) );
}
void UGameHUDWidget::HandleGameEnded( EGameResult Result )
{
	if ( ActiveOverlay )
	{
		ActiveOverlay->RemoveFromParent();
		ActiveOverlay = nullptr;
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

void UGameHUDWidget::ShowTooltipForBuilding( TSubclassOf<ABuilding> buildingClass )
{
	if ( !buildingClass )
	{
		return;
	}
	const ABuilding* cdo = buildingClass->GetDefaultObject<ABuilding>();
	if ( !cdo )
	{
		return;
	}

	if ( cdo->IsA<ADefensiveBuilding>() )
	{
		if ( ActiveEconomyTooltip )
		{
			ActiveEconomyTooltip->HideTooltip();
		}
		if ( ActiveDefensiveTooltip )
		{
			ActiveDefensiveTooltip->ShowTooltip( buildingClass );
		}
	}
	else
	{
		if ( ActiveDefensiveTooltip )
		{
			ActiveDefensiveTooltip->HideTooltip();
		}
		if ( ActiveEconomyTooltip )
		{
			ActiveEconomyTooltip->ShowTooltip( buildingClass );
		}
	}
}

void UGameHUDWidget::InitializeTooltipWidget(
    TSubclassOf<UBuildingTooltipWidget> TooltipClass, TObjectPtr<UBuildingTooltipWidget>& OutTooltip
)
{
	if ( TooltipClass && !OutTooltip )
	{
		OutTooltip = CreateWidget<UBuildingTooltipWidget>( this, TooltipClass );
		if ( OutTooltip )
		{
			OutTooltip->AddToViewport( 99 );
			OutTooltip->ForceHide();
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
	UE_LOG( LogTemp, Warning, TEXT( "UGameHUDWidget::HandleSelectionChanged " ) );
	UpdateExtraButtonsVisibility();

	UCoreManager* coreManager = UCoreManager::Get( this );
	if ( !coreManager )
	{
		return;
	}

	USelectionManagerComponent* selectionManager = coreManager->GetSelectionManager();
	if ( !selectionManager )
	{
		return;
	}

	ABuilding* selectedBuilding = selectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding )
	{
		return;
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

	SelectionManager = InSelectionManager;
	if ( !SelectionManager )
	{
		UE_LOG( LogTemp, Error, TEXT( "InitSelectionManager: SelectionManager is null" ) );
		return;
	}

	SelectionManager->OnSelectionChanged.AddDynamic( this, &UGameHUDWidget::HandleSelectionChanged );

	UE_LOG( LogTemp, Warning, TEXT( "InitSelectionManager: subscribed to OnSelectionChanged" ) );

	HandleSelectionChanged();
}
