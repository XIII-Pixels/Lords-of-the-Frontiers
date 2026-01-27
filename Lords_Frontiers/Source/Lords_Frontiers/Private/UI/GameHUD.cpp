#include "Lords_Frontiers/Public/UI/GameHUD.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Resources/ResourceManager.h"

#include "Components/GridPanel.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
	}
	if ( ButtonBuildingStrawHouse )
	{
		ButtonBuildingStrawHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildStrawHouseClicked );
	}
	if ( ButtonBuildingFarm )
	{
		ButtonBuildingFarm->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildFarmClicked );
	}
	if ( ButtonBuildingLawnHouse )
	{
		ButtonBuildingLawnHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildLawnHouseClicked );
	}
	if ( ButtonBuildingMagicHouse )
	{
		ButtonBuildingMagicHouse->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildMagicHouseClicked );
	}

	if ( ButtonBuildingWall )
	{
		ButtonBuildingWall->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildWallClicked );
	}
	if ( ButtonBuildingTowerT0 )
	{
		ButtonBuildingTowerT0->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerT0Clicked );
	}
	if ( ButtonBuildingTowerT1 )
	{
		ButtonBuildingTowerT1->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerT1Clicked );
	}
	if ( ButtonBuildingTowerT2 )
	{
		ButtonBuildingTowerT2->OnClicked.AddDynamic( this, &UGameHUDWidget::OnBuildTowerT2Clicked );
	}

	if ( UCoreManager* Core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* GL = Core->GetGameLoop() )
		{
			GL->OnPhaseChanged.AddDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			GL->OnBuildTurnChanged.AddDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			GL->OnCombatTimerUpdated.AddDynamic( this, &UGameHUDWidget::HandleCombatTimer );
		}

		if ( UResourceManager* RM = Core->GetResourceManager() )
		{
			RM->OnResourceChanged.AddDynamic( this, &UGameHUDWidget::HandleResourceChanged );
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
	if ( ButtonBuildingWall )
		ButtonBuildingWall->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildWallClicked );
	if ( ButtonBuildingTowerT0 )
		ButtonBuildingTowerT0->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT0Clicked );
	if ( ButtonBuildingTowerT1 )
		ButtonBuildingTowerT1->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT1Clicked );
	if ( ButtonBuildingTowerT2 )
		ButtonBuildingTowerT2->OnClicked.RemoveDynamic( this, &UGameHUDWidget::OnBuildTowerT2Clicked );

	if ( UCoreManager* Core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* GL = Core->GetGameLoop() )
		{
			GL->OnPhaseChanged.RemoveDynamic( this, &UGameHUDWidget::HandlePhaseChanged );
			GL->OnBuildTurnChanged.RemoveDynamic( this, &UGameHUDWidget::HandleTurnChanged );
			GL->OnCombatTimerUpdated.RemoveDynamic( this, &UGameHUDWidget::HandleCombatTimer );
		}

		if ( UResourceManager* RM = Core->GetResourceManager() )
		{
			RM->OnResourceChanged.RemoveDynamic( this, &UGameHUDWidget::HandleResourceChanged );
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
		int32 Seconds = FMath::CeilToInt( TimeRemaining );
		TextTimer->SetText( FText::FromString( FString::Printf( TEXT( "%d" ), Seconds ) ) );
	}
}

void UGameHUDWidget::HandleResourceChanged( EResourceType Type, int32 NewAmount )
{
	UE_LOG( LogTemp, Warning, TEXT( "HandleResourceChanged: Type=%d, Amount=%d" ), (int32) Type, NewAmount );
	UpdateResources();
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

void UGameHUDWidget::UpdateDayText()
{
	if ( !TextDay )
		return;

	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
		return;

	UGameLoopManager* GL = Core->GetGameLoop();
	if ( !GL )
		return;

	int32 Wave = GL->GetCurrentWave();
	TextDay->SetText( FText::FromString( FString::Printf( TEXT( "День %d" ), Wave ) ) );
}

void UGameHUDWidget::UpdateStatusText()
{
	if ( !TextStatus )
		return;

	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
		return;

	UGameLoopManager* GL = Core->GetGameLoop();
	if ( !GL )
		return;

	FText Status;
	EGameLoopPhase Phase = GL->GetCurrentPhase();

	if ( Phase == EGameLoopPhase::Building )
	{
		int32 Turn = GL->GetCurrentBuildTurn();
		if ( Turn == 1 )
		{
			Status = FText::FromString( TEXT( "Утро" ) );
		}
		else
		{
			Status = FText::FromString( TEXT( "Закат" ) );
		}
	}
	else if ( Phase == EGameLoopPhase::Combat )
	{
		Status = FText::FromString( TEXT( "Бой" ) );
	}
	else if ( Phase == EGameLoopPhase::Victory )
	{
		Status = FText::FromString( TEXT( "Победа!" ) );
	}
	else if ( Phase == EGameLoopPhase::Defeat )
	{
		Status = FText::FromString( TEXT( "Поражение" ) );
	}
	else
	{
		Status = FText::FromString( TEXT( "---" ) );
	}

	TextStatus->SetText( Status );
}

void UGameHUDWidget::UpdateResources()
{
	UE_LOG( LogTemp, Warning, TEXT( "=== UpdateResources ===" ) );

	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
	{
		UE_LOG( LogTemp, Error, TEXT( "Core is NULL" ) );
		return;
	}

	UResourceManager* RM = Core->GetResourceManager();
	if ( !RM )
	{
		UE_LOG( LogTemp, Error, TEXT( "ResourceManager is NULL" ) );
		return;
	}

	int32 Gold = RM->GetResourceAmount( EResourceType::Gold );
	int32 Food = RM->GetResourceAmount( EResourceType::Food );
	int32 Pop = RM->GetResourceAmount( EResourceType::Population );

	UE_LOG( LogTemp, Warning, TEXT( "Resources: Gold=%d, Food=%d, Pop=%d" ), Gold, Food, Pop );

	if ( Text_Gold )
	{
		Text_Gold->SetText( FText::AsNumber( Gold ) );
	}

	if ( Text_Food )
	{
		Text_Food->SetText( FText::AsNumber( Food ) );
	}

	if ( Text_Citizens )
	{
		Text_Citizens->SetText( FText::AsNumber( Pop ) );
	}
}

void UGameHUDWidget::UpdateButtonVisibility()
{
	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
		return;

	UGameLoopManager* GL = Core->GetGameLoop();
	if ( !GL )
		return;

	EGameLoopPhase Phase = GL->GetCurrentPhase();

	if ( ButtonEndTurn )
	{
		bool bShow = ( Phase == EGameLoopPhase::Building );
		ButtonEndTurn->SetVisibility( bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}
}

void UGameHUDWidget::OnEndTurnClicked()
{
	if ( UCoreManager* Core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* GL = Core->GetGameLoop() )
		{
			GL->EndBuildTurn();
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
	bShowingEconomyBuildings = true;

	if ( EconomyCardBox )
	{
		EconomyCardBox->SetVisibility( ESlateVisibility::Visible );
	}

	if ( DefensiveCardBox )
	{
		DefensiveCardBox->SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UGameHUDWidget::ShowDefensiveBuildings()
{
	bShowingEconomyBuildings = false;

	if ( EconomyCardBox )
	{
		EconomyCardBox->SetVisibility( ESlateVisibility::Collapsed );
	}

	if ( DefensiveCardBox )
	{
		DefensiveCardBox->SetVisibility( ESlateVisibility::Visible );
	}
}

void UGameHUDWidget::StartBuilding( TSubclassOf<ABuilding> BuildingClass )
{
	if ( !BuildingClass )
	{
		UE_LOG( LogTemp, Warning, TEXT( "StartBuilding: BuildingClass is null" ) );
		return;
	}

	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
	{
		return;
	}

	ABuildManager* BM = Core->GetBuildManager();
	if ( !BM )
	{
		UE_LOG( LogTemp, Warning, TEXT( "StartBuilding: BuildManager is null" ) );
		return;
	}

	BM->StartPlacingBuilding( BuildingClass );
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

void UGameHUDWidget::OnBuildWallClicked()
{
	StartBuilding( WallClass );
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
	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
		return;

	UGameLoopManager* GL = Core->GetGameLoop();
	if ( !GL )
		return;

	EGameLoopPhase Phase = GL->GetCurrentPhase();
	bool bShowBuildingUI = ( Phase == EGameLoopPhase::Building );

	ESlateVisibility NewVisibility = bShowBuildingUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	// Кнопки категорий
	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->SetVisibility( NewVisibility );
	}
	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->SetVisibility( NewVisibility );
	}
	if ( ButtonClassBuilding3 )
	{
		ButtonClassBuilding3->SetVisibility( NewVisibility );
	}
	if ( ButtonClassBuilding4 )
	{
		ButtonClassBuilding4->SetVisibility( NewVisibility );
	}

	if ( ButtonRelocateBuilding )
	{
		ButtonRelocateBuilding->SetVisibility( NewVisibility );
	}
	if ( ButtonRemoveBuilding )
	{
		ButtonRemoveBuilding->SetVisibility( NewVisibility );
	}

	if ( bShowBuildingUI )
	{
		if ( bShowingEconomyBuildings )
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
	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
		return;

	ABuildManager* BM = Core->GetBuildManager();
	if ( !BM )
		return;

	if ( BM->IsPlacing() )
	{
		BM->CancelPlacing();
	}
}