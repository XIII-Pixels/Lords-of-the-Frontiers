#include "Lords_Frontiers/Public/UI/GameHUD.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Resources/ResourceManager.h"

#include "Camera/CameraComponent.h"
#include "Components/GridPanel.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
	if ( pc )
	{
		UResourceManager* rM = pc->FindComponentByClass<UResourceManager>();
		if ( !rM )
		{
			rM = NewObject<UResourceManager>( pc, TEXT( "GlobalResourceManager" ) );
			rM->RegisterComponent();
		}
		rM->OnResourceChanged.RemoveDynamic( this, &UGameHUDWidget::HandleResourceChanged );
		rM->OnResourceChanged.AddDynamic( this, &UGameHUDWidget::HandleResourceChanged );
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
		}
	}

	if ( TextTimer )
	{
		TextTimer->SetVisibility( ESlateVisibility::Collapsed );
	}

	UpdateDayText();
	UpdateStatusText();
	UpdateResources();
	UpdateButtonVisibility();
	UpdateBuildingUIVisibility();

	ShowEconomyBuildings();

	UpdateResources();
	UpdateAllBuildingButtons();
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

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.RemoveDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			gL->OnBuildTurnChanged.RemoveDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			gL->OnCombatTimerUpdated.RemoveDynamic( this, &UGameHUDWidget::HandleCombatTimer );
		}

		if ( UResourceManager* rM = core->GetResourceManager() )
		{
			rM->OnResourceChanged.RemoveDynamic( this, &UGameHUDWidget::HandleResourceChanged );
		}
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
	UE_LOG( LogTemp, Warning, TEXT( "HandleResourceChanged: Type=%d, Amount=%d" ), (int32) Type, NewAmount );
	UpdateResources();
	UpdateAllBuildingButtons();
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

	if ( ActiveBonusIcons_.Num() > 0 )
	{
		UpdateBonusIconPositions();
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

	const float baseOrthoWidth = 1048.0f;
	const float baseScale = 0.7f;
	const float scale = FMath::Clamp( ( baseOrthoWidth / orthoWidth ) * baseScale, 0.3f, 2.0f );

	for ( int32 i = 0; i < ActiveBonusIcons_.Num(); ++i )
	{
		if ( !ActiveBonusIcons_[i] )
		{
			continue;
		}
		if ( !ActiveBonusWorldPositions_.IsValidIndex( i ) )
		{
			continue;
		}

		FVector iconWorldPos = ActiveBonusWorldPositions_[i] + FVector( 0.0f, 0.0f, 40.0f );

		FVector2D screenPos;
		if ( !pc->ProjectWorldLocationToScreen( iconWorldPos, screenPos ) )
		{
			ActiveBonusIcons_[i]->SetVisibility( ESlateVisibility::Collapsed );
			continue;
		}

		screenPos /= viewportScale;

		ActiveBonusIcons_[i]->SetRenderScale( FVector2D( scale, scale ) );

		UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>( ActiveBonusIcons_[i]->Slot.Get() );
		if ( !slot )
		{
			continue;
		}

		slot->SetPosition( screenPos );
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
	if ( !core )
	{
		UE_LOG( LogTemp, Error, TEXT( "Core is NULL" ) );
		return;
	}

	UResourceManager* rM = core->GetResourceManager();
	if ( !rM )
	{
		UE_LOG( LogTemp, Error, TEXT( "ResourceManager is NULL" ) );
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
		UE_LOG( LogTemp, Warning, TEXT( "StartBuilding: BuildingClass is null" ) );
		return;
	}

	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return;
	}

	ABuildManager* bM = core->GetBuildManager();
	if ( !bM )
	{
		UE_LOG( LogTemp, Warning, TEXT( "StartBuilding: BuildManager is null" ) );
		return;
	}

	bM->StartPlacingBuilding( BuildingClass );
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
	if ( !core )
	{
		return;
	}
	ABuildManager* bM = core->GetBuildManager();
	if ( !bM )
	{
		return;
	};

	if ( bM->IsPlacing() )
	{
		bM->CancelPlacing();
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

	button->SetIsEnabled( bCanAfford );

	button->SetRenderOpacity( bCanAfford ? 1.0f : 0.4f );

	button->SetBackgroundColor( bCanAfford ? AffordableColor : TooExpensiveColor );
}

void UGameHUDWidget::StartTooltipTimer( TSubclassOf<ABuilding> buildingClass )
{
	PendingBuildingClass = buildingClass;
	GetWorld()->GetTimerManager().SetTimer(
	    TooltipTimerHandle, this, &UGameHUDWidget::ShowTooltipInternal, 0.5f, false
	);
}

void UGameHUDWidget::OnBuildingUnhovered()
{
	GetWorld()->GetTimerManager().ClearTimer( TooltipTimerHandle );
	if ( ActiveTooltip )
		ActiveTooltip->SetVisibility( ESlateVisibility::Collapsed );
	PendingBuildingClass = nullptr;
}

void UGameHUDWidget::ShowTooltipInternal()
{
	if ( !PendingBuildingClass || !TooltipClass )
	{
		return;
	}

	if ( !ActiveTooltip )
	{
		ActiveTooltip = CreateWidget<UBuildingTooltipWidget>( this, TooltipClass );
		ActiveTooltip->AddToViewport( 99 );
	}

	ActiveTooltip->UpdateTooltip( PendingBuildingClass->GetDefaultObject<ABuilding>() );
	ActiveTooltip->SetVisibility( ESlateVisibility::HitTestInvisible );

	FVector2D mousePos;
	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		pc->GetMousePosition( mousePos.X, mousePos.Y );

		FVector2D viewportSize;
		if ( GEngine && GEngine->GameViewport )
		{
			GEngine->GameViewport->GetViewportSize( viewportSize );
		}

		float offsetYa = ( mousePos.Y > ( viewportSize.Y / 2.f ) ) ? -180.f : 25.f;

		ActiveTooltip->SetPositionInViewport( mousePos + FVector2D( 25, offsetYa ) );
	}
}