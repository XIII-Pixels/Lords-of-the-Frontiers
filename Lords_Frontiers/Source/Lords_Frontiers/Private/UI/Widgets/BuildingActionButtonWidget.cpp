#include "Lords_Frontiers/Public/UI/Widgets/BuildingActionButtonWidget.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Localization/GameLocalization.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"
#include "sound/SoundEffectManager.h"

static FText FormatResourceAmount( const FResourceProduction& amount )
{
	const int32 values[] = { amount.Gold, amount.Food, amount.Population, amount.Progress };

	FString result;
	for ( int32 value : values )
	{
		if ( value != 0 )
		{
			if ( !result.IsEmpty() )
			{
				result += TEXT( " / " );
			}
			result += FString::FromInt( value );
		}
	}

	if ( result.IsEmpty() )
	{
		result = TEXT( "0" );
	}

	return FText::FromString( result );
}

void UBuildingActionButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Button )
	{
		Button->OnClicked.AddDynamic( this, &UBuildingActionButtonWidget::HandleClicked );
		Button->OnHovered.AddDynamic( this, &UBuildingActionButtonWidget::HandleHovered );
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
}

void UBuildingActionButtonWidget::NativeDestruct()
{
	if ( Button )
	{
		Button->OnClicked.RemoveDynamic( this, &UBuildingActionButtonWidget::HandleClicked );
		Button->OnHovered.RemoveDynamic( this, &UBuildingActionButtonWidget::HandleHovered );
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

void UBuildingActionButtonWidget::RefreshFor( ABuilding* building )
{
	Building_ = building;

	const bool bCanAct = IsValid( building ) && CanActOn( building );
	SetVisibility( bCanAct ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	if ( !bCanAct )
	{
		return;
	}

	const bool bCombat = IsCombatPhase();
	if ( Button )
	{
		Button->SetIsEnabled( !bCombat );
		Button->SetRenderOpacity( bCombat ? DisabledOpacity : 1.0f );
	}

	if ( LabelText )
	{
		LabelText->SetText(
		    Action == EBuildingActionType::Relocate ? LF_LOC( "Building.Action.Relocate" )
		                                            : LF_LOC( "Building.Action.Remove" ) );
	}

	ApplyCostText( building );
}

bool UBuildingActionButtonWidget::CanActOn( const ABuilding* building ) const
{
	if ( !building )
	{
		return false;
	}

	return Action == EBuildingActionType::Relocate ? building->CanBeRelocated() : building->CanBeRemoved();
}

void UBuildingActionButtonWidget::ApplyCostText( const ABuilding* building )
{
	if ( !CostText )
	{
		return;
	}

	if ( Action == EBuildingActionType::Relocate )
	{
		CostText->SetText( FText::AsNumber( building->GetRelocationGoldCost() ) );
	}
	else
	{
		CostText->SetText( FormatResourceAmount( building->GetDemolitionRefund() ) );
	}
}

void UBuildingActionButtonWidget::PerformAction( ABuilding* building )
{
	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return;
	}

	USelectionManagerComponent* selectionManager = core->GetSelectionManager();
	ABuildManager* buildManager = core->GetBuildManager();
	UResourceManager* resourceManager = core->GetResourceManager();
	if ( !selectionManager || !buildManager || !resourceManager )
	{
		return;
	}

	if ( Action == EBuildingActionType::Relocate )
	{
		const int32 cost = building->GetRelocationGoldCost();
		if ( !resourceManager->HasEnoughResource( EResourceType::Gold, cost ) )
		{
			return;
		}

		buildManager->StartRelocatingBuilding( building );
		selectionManager->ClearSelection();
	}
	else
	{
		const FResourceProduction refund = building->GetDemolitionRefund();
		if ( !buildManager->RemoveExistingBuilding( building ) )
		{
			return;
		}

		resourceManager->AddResources( refund );
		selectionManager->ClearSelection();
	}
}

bool UBuildingActionButtonWidget::IsCombatPhase() const
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gameLoop = core->GetGameLoop() )
		{
			return gameLoop->GetCurrentPhase() == EGameLoopPhase::Combat;
		}
	}
	return false;
}

FGameplayTag UBuildingActionButtonWidget::GetClickSoundTag() const
{
	return Action == EBuildingActionType::Relocate ? AudioTags::SFX_UI_BUTTON_MOVEBUILDING_CLICKED
	                                               : AudioTags::SFX_UI_BUTTON_DEMOLISHBUILDING_CLICKED;
}

FGameplayTag UBuildingActionButtonWidget::GetHoverSoundTag() const
{
	return Action == EBuildingActionType::Relocate ? AudioTags::SFX_UI_BUTTON_MOVEBUILDING_HOVERED
	                                               : AudioTags::SFX_UI_BUTTON_DEMOLISHBUILDING_HOVERED;
}

void UBuildingActionButtonWidget::HandleClicked()
{
	OnAudioEvent_.Broadcast( { GetClickSoundTag() } );

	ABuilding* building = Building_.Get();
	if ( !IsValid( building ) || IsCombatPhase() || !CanActOn( building ) )
	{
		return;
	}

	PerformAction( building );
}

void UBuildingActionButtonWidget::HandleHovered()
{
	OnAudioEvent_.Broadcast( { GetHoverSoundTag() } );
}
