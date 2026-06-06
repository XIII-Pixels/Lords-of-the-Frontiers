#include "Lords_Frontiers/Public/UI/Widgets/HUDBuildingPanelWidget.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Building/DefensiveBuilding.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "Localization/GameLocalization.h"
#include "Resources/ResourceManager.h"
#include "UI/Widgets/BuildingButtonWidget.h"
#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "sound/SoundEffectManager.h"

void UHUDBuildingPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnDefensiveBuildingsClicked );
		ButtonDefensiveBuildings->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverDefensiveBuildings );
	}
	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnEconomyBuildingClicked );
		ButtonEconomyBuilding->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverEconomyBuilding );
	}

	if ( ButtonBuildingWoodenHouse )
	{
		ButtonBuildingWoodenHouse->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildWoodenHouseClicked );
		ButtonBuildingWoodenHouse->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverWoodenHouse );
		ButtonBuildingWoodenHouse->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStoneHouse )
	{
		ButtonBuildingStoneHouse->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildStoneHouseClicked );
		ButtonBuildingStoneHouse->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverStoneHouse );
		ButtonBuildingStoneHouse->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingGrainField )
	{
		ButtonBuildingGrainField->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildGrainFieldClicked );
		ButtonBuildingGrainField->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverGrainField );
		ButtonBuildingGrainField->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingWheatMill )
	{
		ButtonBuildingWheatMill->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildWheatMillClicked );
		ButtonBuildingWheatMill->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverWheatMill );
		ButtonBuildingWheatMill->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMagicHouse )
	{
		ButtonBuildingMagicHouse->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildMagicHouseClicked );
		ButtonBuildingMagicHouse->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverMagicHouse );
		ButtonBuildingMagicHouse->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}

	if ( ButtonBuildingWoodWall )
	{
		ButtonBuildingWoodWall->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildWoodWallClicked );
		ButtonBuildingWoodWall->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverWoodWall );
		ButtonBuildingWoodWall->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStoneWall )
	{
		ButtonBuildingStoneWall->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildStoneWallClicked );
		ButtonBuildingStoneWall->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverStoneWall );
		ButtonBuildingStoneWall->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT0 )
	{
		ButtonBuildingTowerT0->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerT0Clicked );
		ButtonBuildingTowerT0->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerT0 );
		ButtonBuildingTowerT0->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT1 )
	{
		ButtonBuildingTowerT1->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerT1Clicked );
		ButtonBuildingTowerT1->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerT1 );
		ButtonBuildingTowerT1->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT2 )
	{
		ButtonBuildingTowerT2->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerT2Clicked );
		ButtonBuildingTowerT2->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerT2 );
		ButtonBuildingTowerT2->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMortira )
	{
		ButtonBuildingMortira->OnClicked.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerMortiraClicked );
		ButtonBuildingMortira->OnHovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerMortira );
		ButtonBuildingMortira->OnUnhovered.AddDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}

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

	FWidgetAnimationDynamicEvent animFinished;
	animFinished.BindDynamic( this, &UHUDBuildingPanelWidget::OnVisibilityAnimFinished );
	if ( ShowAnim )
	{
		BindToAnimationFinished( ShowAnim, animFinished );
	}
	if ( HideAnim )
	{
		BindToAnimationFinished( HideAnim, animFinished );
	}

	TrySubscribeManagers();

	ShowEconomyBuildings();
	UpdateAllBuildingButtons();

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			SetBuildingUIVisible( gL->GetCurrentPhase() == EGameLoopPhase::Building );
		}
	}
}

void UHUDBuildingPanelWidget::NativeDestruct()
{
	if ( ButtonDefensiveBuildings )
	{
		ButtonDefensiveBuildings->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnDefensiveBuildingsClicked );
		ButtonDefensiveBuildings->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverDefensiveBuildings );
	}
	if ( ButtonEconomyBuilding )
	{
		ButtonEconomyBuilding->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnEconomyBuildingClicked );
		ButtonEconomyBuilding->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverEconomyBuilding );
	}

	if ( ButtonBuildingWoodenHouse )
	{
		ButtonBuildingWoodenHouse->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildWoodenHouseClicked );
		ButtonBuildingWoodenHouse->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverWoodenHouse );
		ButtonBuildingWoodenHouse->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStoneHouse )
	{
		ButtonBuildingStoneHouse->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildStoneHouseClicked );
		ButtonBuildingStoneHouse->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverStoneHouse );
		ButtonBuildingStoneHouse->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingGrainField )
	{
		ButtonBuildingGrainField->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildGrainFieldClicked );
		ButtonBuildingGrainField->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverGrainField );
		ButtonBuildingGrainField->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingWheatMill )
	{
		ButtonBuildingWheatMill->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildWheatMillClicked );
		ButtonBuildingWheatMill->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverWheatMill );
		ButtonBuildingWheatMill->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMagicHouse )
	{
		ButtonBuildingMagicHouse->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildMagicHouseClicked );
		ButtonBuildingMagicHouse->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverMagicHouse );
		ButtonBuildingMagicHouse->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}

	if ( ButtonBuildingWoodWall )
	{
		ButtonBuildingWoodWall->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildWoodWallClicked );
		ButtonBuildingWoodWall->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverWoodWall );
		ButtonBuildingWoodWall->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingStoneWall )
	{
		ButtonBuildingStoneWall->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildStoneWallClicked );
		ButtonBuildingStoneWall->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverStoneWall );
		ButtonBuildingStoneWall->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT0 )
	{
		ButtonBuildingTowerT0->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerT0Clicked );
		ButtonBuildingTowerT0->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerT0 );
		ButtonBuildingTowerT0->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT1 )
	{
		ButtonBuildingTowerT1->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerT1Clicked );
		ButtonBuildingTowerT1->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerT1 );
		ButtonBuildingTowerT1->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingTowerT2 )
	{
		ButtonBuildingTowerT2->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerT2Clicked );
		ButtonBuildingTowerT2->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerT2 );
		ButtonBuildingTowerT2->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}
	if ( ButtonBuildingMortira )
	{
		ButtonBuildingMortira->OnClicked.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildTowerMortiraClicked );
		ButtonBuildingMortira->OnHovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnHoverTowerMortira );
		ButtonBuildingMortira->OnUnhovered.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnBuildingUnhovered );
	}

	UnsubscribeManagers();

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

bool UHUDBuildingPanelWidget::TrySubscribeManagers()
{
	if ( bIsSubscribed_ )
	{
		return true;
	}

	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return false;
	}

	if ( UResourceManager* rM = core->GetResourceManager() )
	{
		rM->OnResourceChanged.AddUniqueDynamic( this, &UHUDBuildingPanelWidget::HandleResourceChanged );
	}

	if ( UGameLoopManager* gL = core->GetGameLoop() )
	{
		gL->OnPhaseChanged.AddUniqueDynamic( this, &UHUDBuildingPanelWidget::HandlePhaseChanged );
	}

	if ( ABuildManager* bM =
	         Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) ) )
	{
		bM->OnPlacingStarted.AddUniqueDynamic( this, &UHUDBuildingPanelWidget::OnPlacingStarted );
		bM->OnPlacingCancelled.AddUniqueDynamic( this, &UHUDBuildingPanelWidget::OnPlacingCancelled );
	}

	bIsSubscribed_ = true;
	return true;
}

void UHUDBuildingPanelWidget::UnsubscribeManagers()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UResourceManager* rM = core->GetResourceManager() )
		{
			rM->OnResourceChanged.RemoveDynamic( this, &UHUDBuildingPanelWidget::HandleResourceChanged );
		}
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->OnPhaseChanged.RemoveDynamic( this, &UHUDBuildingPanelWidget::HandlePhaseChanged );
		}
	}

	if ( ABuildManager* bM =
	         Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( GetWorld(), ABuildManager::StaticClass() ) ) )
	{
		bM->OnPlacingStarted.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnPlacingStarted );
		bM->OnPlacingCancelled.RemoveDynamic( this, &UHUDBuildingPanelWidget::OnPlacingCancelled );
	}

	bIsSubscribed_ = false;
}

void UHUDBuildingPanelWidget::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	SetBuildingUIVisible( NewPhase == EGameLoopPhase::Building );

	if ( NewPhase == EGameLoopPhase::Combat )
	{
		CancelCurrentBuilding();
	}
}

void UHUDBuildingPanelWidget::HandleResourceChanged( EResourceType Type, int32 NewAmount )
{
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

void UHUDBuildingPanelWidget::RefreshAvailability()
{
	UpdateAllBuildingButtons();
}

void UHUDBuildingPanelWidget::OnDefensiveBuildingsClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDINGCATEGORY_CLICKED } );
	ShowDefensiveBuildings();
}

void UHUDBuildingPanelWidget::OnEconomyBuildingClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDINGCATEGORY_CLICKED } );
	ShowEconomyBuildings();
}

void UHUDBuildingPanelWidget::ShowEconomyBuildings()
{
	bShowingEconomyBuildings_ = true;

	if ( EconomyCardBox )
	{
		EconomyCardBox->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
	}
	if ( DefensiveCardBox )
	{
		DefensiveCardBox->SetVisibility( ESlateVisibility::Collapsed );
	}

	UpdateCategoryButtonsVisual();
}

void UHUDBuildingPanelWidget::ShowDefensiveBuildings()
{
	bShowingEconomyBuildings_ = false;

	if ( EconomyCardBox )
	{
		EconomyCardBox->SetVisibility( ESlateVisibility::Collapsed );
	}
	if ( DefensiveCardBox )
	{
		DefensiveCardBox->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
	}

	UpdateCategoryButtonsVisual();
}

void UHUDBuildingPanelWidget::StartBuilding( TSubclassOf<ABuilding> BuildingClass )
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
				GEngine->AddOnScreenDebugMessage(
				    -1, 2.f, FColor::Red, LF_LOC( "Build.NotEnoughResources" ).ToString()
				);
			}
			return;
		}
	}

	bM->StartPlacingBuilding( BuildingClass );

	bIsBuildingLocked = true;
	LockedBuildingClass = BuildingClass;
}

void UHUDBuildingPanelWidget::OnBuildWoodenHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingWoodenHouse );
	StartBuilding( WoodenHouseClass );
}

void UHUDBuildingPanelWidget::OnBuildStoneHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingStoneHouse );
	StartBuilding( StoneHouseClass );
}

void UHUDBuildingPanelWidget::OnBuildGrainFieldClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingGrainField );
	StartBuilding( GrainFieldClass );
}

void UHUDBuildingPanelWidget::OnBuildWheatMillClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingWheatMill );
	StartBuilding( WheatMillClass );
}

void UHUDBuildingPanelWidget::OnBuildMagicHouseClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingMagicHouse );
	StartBuilding( MagicHouseClass );
}

void UHUDBuildingPanelWidget::OnBuildWoodWallClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingWoodWall );
	StartBuilding( WoodWallClass );
}

void UHUDBuildingPanelWidget::OnBuildStoneWallClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingStoneWall );
	StartBuilding( StoneWallClass );
}

void UHUDBuildingPanelWidget::OnBuildTowerT0Clicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingTowerT0 );
	StartBuilding( TowerT0Class );
}

void UHUDBuildingPanelWidget::OnBuildTowerT1Clicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingTowerT1 );
	StartBuilding( TowerT1Class );
}

void UHUDBuildingPanelWidget::OnBuildTowerT2Clicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingTowerT2 );
	StartBuilding( TowerT2Class );
}

void UHUDBuildingPanelWidget::OnBuildTowerMortiraClicked()
{
	PlayOnBuildingButtonClickedSound( ButtonBuildingMortira );
	StartBuilding( TowerMortiraClass );
}

void UHUDBuildingPanelWidget::SetBuildingUIVisible( bool bVisible )
{
	bWantsVisible_ = bVisible;

	if ( bVisible )
	{
		if ( BackForButton )
		{
			BackForButton->SetVisibility( ESlateVisibility::Visible );
		}
		if ( ButtonEconomyBuilding )
		{
			ButtonEconomyBuilding->SetVisibility( ESlateVisibility::Visible );
		}
		if ( ButtonDefensiveBuildings )
		{
			ButtonDefensiveBuildings->SetVisibility( ESlateVisibility::Visible );
		}

		if ( bShowingEconomyBuildings_ )
		{
			ShowEconomyBuildings();
		}
		else
		{
			ShowDefensiveBuildings();
		}

		SetVisibility( ESlateVisibility::Visible );
	}

	PlayVisibilityAnim( bVisible );
}

void UHUDBuildingPanelWidget::PlayVisibilityAnim( bool bVisible )
{
	if ( bVisible )
	{
		if ( ShowAnim )
		{
			PlayAnimationForward( ShowAnim );
		}
		else if ( HideAnim )
		{
			PlayAnimationReverse( HideAnim );
		}
	}
	else
	{
		if ( HideAnim )
		{
			PlayAnimationForward( HideAnim );
		}
		else if ( ShowAnim )
		{
			PlayAnimationReverse( ShowAnim );
		}
		else
		{
			SetVisibility( ESlateVisibility::Collapsed );
		}
	}
}

void UHUDBuildingPanelWidget::OnVisibilityAnimFinished()
{
	if ( !bWantsVisible_ )
	{
		SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UHUDBuildingPanelWidget::CancelCurrentBuilding()
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

void UHUDBuildingPanelWidget::OnPlacingStarted()
{
	bIsPlacingActive_ = true;
	SetBuildingUIVisible( false );
}

void UHUDBuildingPanelWidget::OnPlacingCancelled()
{
	bIsPlacingActive_ = false;
	bIsBuildingLocked = false;
	LockedBuildingClass = nullptr;

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			if ( gL->GetCurrentPhase() == EGameLoopPhase::Building )
			{
				SetBuildingUIVisible( true );
			}
		}
	}

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

	HideTooltipForBuilding();
}

void UHUDBuildingPanelWidget::UpdateCategoryButtonsVisual()
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

void UHUDBuildingPanelWidget::UpdateAllBuildingButtons()
{
	UpdateButtonAvailability( ButtonBuildingWoodenHouse, WoodenHouseClass );
	UpdateButtonAvailability( ButtonBuildingStoneHouse, StoneHouseClass );
	UpdateButtonAvailability( ButtonBuildingGrainField, GrainFieldClass );
	UpdateButtonAvailability( ButtonBuildingWheatMill, WheatMillClass );
	UpdateButtonAvailability( ButtonBuildingMagicHouse, MagicHouseClass );

	UpdateButtonAvailability( ButtonBuildingWoodWall, WoodWallClass );
	UpdateButtonAvailability( ButtonBuildingStoneWall, StoneWallClass );
	UpdateButtonAvailability( ButtonBuildingTowerT0, TowerT0Class );
	UpdateButtonAvailability( ButtonBuildingTowerT1, TowerT1Class );
	UpdateButtonAvailability( ButtonBuildingTowerT2, TowerT2Class );
	UpdateButtonAvailability( ButtonBuildingMortira, TowerMortiraClass );
}

void UHUDBuildingPanelWidget::UpdateButtonAvailability(
    UBuildingButtonWidget* button, TSubclassOf<ABuilding> buildingClass
)
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

void UHUDBuildingPanelWidget::OnBuildingUnhovered()
{
	if ( bIsBuildingLocked && LockedBuildingClass )
	{
		ShowTooltipForBuilding( LockedBuildingClass );
		return;
	}

	// When an object is selected, dismiss the build tooltip instantly (no
	// hide-out animation) as the mouse leaves the button.
	HideTooltipForBuilding( !IsBuildingSelected() );
}

void UHUDBuildingPanelWidget::PlayOnBuildingButtonClickedSound( const UBuildingButtonWidget* button ) const
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

void UHUDBuildingPanelWidget::PlayOnBuildingButtonHoveredSound( const UBuildingButtonWidget* button ) const
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

UBuildingTooltipWidget* UHUDBuildingPanelWidget::EnsureTooltipForBuilding( const ABuilding* buildingForTypeCheck )
{
	if ( !buildingForTypeCheck )
	{
		HideTooltipForBuilding();
		return nullptr;
	}

	TSubclassOf<UBuildingTooltipWidget> tooltipClass =
	    buildingForTypeCheck->IsA<ADefensiveBuilding>() ? DefensiveTooltipClass : EconomyTooltipClass;

	if ( !tooltipClass )
	{
		HideTooltipForBuilding();
		return nullptr;
	}

	if ( !CurrentTooltip || !CurrentTooltip->IsA( tooltipClass ) )
	{
		if ( CurrentTooltip )
		{
			CurrentTooltip->RemoveFromParent();
			CurrentTooltip = nullptr;
		}

		CurrentTooltip = CreateWidget<UBuildingTooltipWidget>( GetWorld(), tooltipClass );
		if ( !CurrentTooltip )
		{
			return nullptr;
		}

		CurrentTooltip->AddToViewport( 99 );
	}

	CurrentTooltip->SetVisibility( ESlateVisibility::HitTestInvisible );
	return CurrentTooltip;
}

void UHUDBuildingPanelWidget::ShowTooltipForBuilding( TSubclassOf<ABuilding> buildingClass )
{
	OnBuildingButtonHovered.Broadcast();

	// If a building is already selected (its info window is shown), skip the
	// tooltip's intro animation so hovering build buttons doesn't replay it.
	const bool bAnimate = !IsBuildingSelected();

	const ABuilding* cdo = buildingClass ? buildingClass->GetDefaultObject<ABuilding>() : nullptr;
	if ( UBuildingTooltipWidget* tooltip = EnsureTooltipForBuilding( cdo ) )
	{
		tooltip->ShowTooltip( buildingClass, bAnimate );
	}
}

bool UHUDBuildingPanelWidget::IsBuildingSelected() const
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( USelectionManagerComponent* selection = core->GetSelectionManager() )
		{
			return IsValid( selection->GetPrimarySelectedBuilding() );
		}
	}
	return false;
}

void UHUDBuildingPanelWidget::ShowTooltipForBuilding( const ABuilding* building )
{
	if ( UBuildingTooltipWidget* tooltip = EnsureTooltipForBuilding( building ) )
	{
		tooltip->ShowTooltipForBuildingInstance( building );
	}
}

void UHUDBuildingPanelWidget::HideTooltipForBuilding( bool bAnimate )
{
	if ( CurrentTooltip )
	{
		if ( bAnimate )
		{
			CurrentTooltip->HideTooltip();
		}
		else
		{
			CurrentTooltip->ForceHide();
		}
	}
}

void UHUDBuildingPanelWidget::InitializeTooltipWidget(
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
