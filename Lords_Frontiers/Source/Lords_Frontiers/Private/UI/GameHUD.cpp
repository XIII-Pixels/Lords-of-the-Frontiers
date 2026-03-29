#include "Lords_Frontiers/Public/UI/GameHUD.h"

#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/Construction/BuildManager.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"
#include "UI/Widgets/GameStateOverlayWidget.h"
#include "Camera/StrategyCamera.h"
#include "UI/Widgets/BuildingTooltipWidget.h"


#include "Camera/CameraComponent.h"
#include "Components/GridPanel.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

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
	}
	if ( ButtonRemoveBuilding )
	{
		ButtonRemoveBuilding->OnClicked.AddDynamic( this, &UGameHUDWidget::OnRemoveBuildingClicked );
	}
	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->OnClicked.AddDynamic( this, &UGameHUDWidget::OnDefensiveBuildingsClicked );
	}
	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->OnClicked.AddDynamic( this, &UGameHUDWidget::OnEconomyBuildingClicked );
	}
	if ( ButtonEndTurn )
	{
		ButtonEndTurn->OnClicked.AddDynamic( this, &UGameHUDWidget::OnEndTurnClicked );
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
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.AddDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			gL->OnBuildTurnChanged.AddDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			gL->OnCombatTimerUpdated.AddDynamic( this, &UGameHUDWidget::HandleCombatTimer );
			gL->OnGameEnded.AddUniqueDynamic( this, &UGameHUDWidget::HandleGameEnded );
		}
	}

	if ( TextTimer )
	{
		TextTimer->SetVisibility( ESlateVisibility::Collapsed );
	}

	if ( BtnToggleWaveInfo )
	{
		BtnToggleWaveInfo->OnClicked.AddDynamic( this, &UGameHUDWidget::OnWaveInfoButtonClicked );
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
	UpdateWaveInfoButtonVisuals();
}

void UGameHUDWidget::NativeDestruct()
{
	if ( ButtonRelocateBuilding )
		ButtonRelocateBuilding->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnRelocateBuildingClicked );
	if ( ButtonRemoveBuilding )
		ButtonRemoveBuilding->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnRemoveBuildingClicked );
	if ( ButtonDefensiveBuildings )
		ButtonDefensiveBuildings->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnDefensiveBuildingsClicked );
	if ( ButtonEconomyBuilding )
		ButtonEconomyBuilding->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnEconomyBuildingClicked );
	if ( ButtonEndTurn )
		ButtonEndTurn->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnEndTurnClicked );

	if ( ButtonBuildingWoodenHouse )
		ButtonBuildingWoodenHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildWoodenHouseClicked );
	if ( ButtonBuildingStrawHouse )
		ButtonBuildingStrawHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildStrawHouseClicked );
	if ( ButtonBuildingFarm )
		ButtonBuildingFarm->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildFarmClicked );
	if ( ButtonBuildingLawnHouse )
		ButtonBuildingLawnHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildLawnHouseClicked );
	if ( ButtonBuildingMagicHouse )
		ButtonBuildingMagicHouse->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildMagicHouseClicked );

	if ( ButtonBuildingWoodWall )
		ButtonBuildingWoodWall->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildWoodWallClicked );
	if ( ButtonBuildingStoneWall )
		ButtonBuildingStoneWall->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildStoneWallClicked );
	if ( ButtonBuildingTowerT0 )
		ButtonBuildingTowerT0->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT0Clicked );
	if ( ButtonBuildingTowerT1 )
		ButtonBuildingTowerT1->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT1Clicked );
	if ( ButtonBuildingTowerT2 )
		ButtonBuildingTowerT2->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT2Clicked );
	if ( ButtonBuildingMortira )
		ButtonBuildingMortira->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerMortiraClicked );

	if ( BtnToggleWaveInfo )
		BtnToggleWaveInfo->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnWaveInfoButtonClicked );

	ABuildManager* buildManager =
	    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) );
	if ( buildManager )
	{
		buildManager->OnBonusPreviewUpdated.RemoveDynamic( this, &UGameHUDWidget::HandleBonusPreviewUpdated );
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.RemoveDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			gL->OnBuildTurnChanged.RemoveDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			gL->OnCombatTimerUpdated.RemoveDynamic( this, &UGameHUDWidget::HandleCombatTimer );
			gL->OnGameEnded.RemoveDynamic( this, &UGameHUDWidget::HandleGameEnded );
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

	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( WavePanelAnimationTimerHandle );
	}

	Super::NativeDestruct();
}

void UGameHUDWidget::HandleTurnChanged( int32 CurrentTurn, int32 MaxTurns )
{
	UpdateStatusText();
}

void UGameHUDWidget::HandleCombatTimer( float TimeRemaining, float TotalTime )
{
	if ( TextTimer )
	{
		int32 seconds = FMath::CeilToInt( TimeRemaining );
		TextTimer->SetText( FText::FromString( FString::Printf( TEXT( "%d" ), seconds ) ) );
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

	if ( TextTimer )
	{
		bool bShowTimer = ( NewPhase == EGameLoopPhase::Combat );
		TextTimer->SetVisibility( bShowTimer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}

	if ( NewPhase == EGameLoopPhase::Combat )
	{
		CancelCurrentBuilding();
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

	float orthoWidth = 2048.0f;
	if ( pc->PlayerCameraManager )
	{
		AActor* viewTarget = pc->PlayerCameraManager->GetViewTarget();
		if ( viewTarget )
		{
			UCameraComponent* cam = viewTarget->FindComponentByClass<UCameraComponent>();
			if ( cam )
			{
				orthoWidth = cam->OrthoWidth;
			}
		}
	}

	const float baseOrthoWigth = 2048.0f;
	const float baseScale = 0.5f;
	const float scale =
	    FMath::Clamp( ( baseOrthoWigth / orthoWidth ) * BaseBonusIconScale, MinBonusIconScale, MaxBonusIconScale );
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

		if ( scale <= MinBonusIconScale + 0.01f )
		{
			ActiveBonusIcons_[i]->SetVisibility( ESlateVisibility::Collapsed );
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

		ActiveBonusIcons_[i]->SetRenderScale( FVector2D( scale, scale ) );
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
		return;

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
}

void UGameHUDWidget::OnRelocateBuildingClicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Relocate building clicked" ) );
}

void UGameHUDWidget::OnRemoveBuildingClicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Remove building clicked" ) );
}

void UGameHUDWidget::OnDefensiveBuildingsClicked()
{
	ShowDefensiveBuildings();
}

void UGameHUDWidget::OnEconomyBuildingClicked()
{
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
	StartBuilding( WoodenHouseClass );
}

void UGameHUDWidget::OnBuildStrawHouseClicked()
{
	StartBuilding( StrawHouseClass );
}

void UGameHUDWidget::OnBuildFarmClicked()
{
	StartBuilding( FarmClass );
}

void UGameHUDWidget::OnBuildLawnHouseClicked()
{
	StartBuilding( LawnHouseClass );
}

void UGameHUDWidget::OnBuildMagicHouseClicked()
{
	StartBuilding( MagicHouseClass );
}

void UGameHUDWidget::OnBuildWoodWallClicked()
{
	StartBuilding( WoodWallClass );
}

void UGameHUDWidget::OnBuildStoneWallClicked()
{
	StartBuilding( StoneWallClass );
}

void UGameHUDWidget::OnBuildTowerT0Clicked()
{
	StartBuilding( TowerT0Class );
}

void UGameHUDWidget::OnBuildTowerT1Clicked()
{
	StartBuilding( TowerT1Class );
}

void UGameHUDWidget::OnBuildTowerT2Clicked()
{
	StartBuilding( TowerT2Class );
}

void UGameHUDWidget::OnBuildTowerMortiraClicked()
{
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
			ActiveEconomyTooltip->HideTooltip();
		if ( ActiveDefensiveTooltip )
			ActiveDefensiveTooltip->HideTooltip();
	}
}

void UGameHUDWidget::ToggleWaveInfoPanel()
{
	if ( !WavePanelClass )
	{
		return;
	}

	if ( bIsWavePanelAnimating )
	{
		return;
	}

	if ( !ActiveWavePanel )
	{
		ActiveWavePanel = CreateWidget<UWaveInfoPanelWidget>( this, WavePanelClass );
		if ( ActiveWavePanel )
		{
			ActiveWavePanel->AddToViewport( 0 );
		}
	}

	if ( !ActiveWavePanel )
	{
		return;
	}

	bIsWavePanelAnimating = true;

	GetWorld()->GetTimerManager().SetTimer(
	    WavePanelAnimationTimerHandle, this, &UGameHUDWidget::UnlockWaveInfoButton, 0.3f, false
	);

	bIsWavePanelOpen = !bIsWavePanelOpen;

	if ( bIsWavePanelOpen )
	{
		UCoreManager* core = UCoreManager::Get( this );
		UGameLoopManager* gameLoop = core ? core->GetGameLoop() : nullptr;
		AWaveManager* waveManager = core ? core->GetWaveManager() : nullptr;

		if ( gameLoop && waveManager )
		{
			int32 waveIndex = gameLoop->GetCurrentWave() - 1;
			TMap<TSubclassOf<AUnit>, int32> waveData = waveManager->GetNextWaveComposition( waveIndex );
			ActiveWavePanel->PopulatePanel( waveData );
		}

		ActiveWavePanel->PlaySlideInAnimation();
	}
	else
	{
		ActiveWavePanel->PlaySlideOutAnimation();
	}

	UpdateWaveInfoButtonVisuals();
}

void UGameHUDWidget::OnWaveInfoButtonClicked()
{
	ToggleWaveInfoPanel();
}

void UGameHUDWidget::UpdateWaveInfoButtonVisuals()
{
	if ( ImgWaveInfoRed )
	{
		ImgWaveInfoRed->SetVisibility(
		    bIsWavePanelOpen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed
		);
	}
	if ( ImgWaveInfoWhite )
	{
		ImgWaveInfoWhite->SetVisibility(
		    bIsWavePanelOpen ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible
		);
	}
}

void UGameHUDWidget::UnlockWaveInfoButton()
{
	bIsWavePanelAnimating = false;
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
void UGameHUDWidget::HandleGameEnded( bool bVictory )
{
	if ( ActiveOverlay )
	{
		ActiveOverlay->RemoveFromParent();
		ActiveOverlay = nullptr;
	}

	TSubclassOf<UGameStateOverlayWidget> ClassToUse = bVictory ? WinWidgetClass : LoseWidgetClass;
	if ( !ClassToUse )
		return;

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
	UCoreManager* core = UCoreManager::Get( this );
	if ( core && core->GetGameLoop() && !core->GetGameLoop()->IsGameStarted() )
	{
		return;
	}

	if ( ActiveOverlay )
	{
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
			return;

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
		return;
	const ABuilding* cdo = buildingClass->GetDefaultObject<ABuilding>();
	if ( !cdo )
		return;

	if ( cdo->IsA<ADefensiveBuilding>() )
	{
		if ( ActiveEconomyTooltip )
			ActiveEconomyTooltip->HideTooltip();
		if ( ActiveDefensiveTooltip )
			ActiveDefensiveTooltip->ShowTooltip( buildingClass );
	}
	else
	{
		if ( ActiveDefensiveTooltip )
			ActiveDefensiveTooltip->HideTooltip();
		if ( ActiveEconomyTooltip )
			ActiveEconomyTooltip->ShowTooltip( buildingClass );
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