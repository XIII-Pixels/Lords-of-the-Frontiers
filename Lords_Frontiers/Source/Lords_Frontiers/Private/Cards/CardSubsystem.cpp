#include "Cards/CardSubsystem.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardPoolConfig.h"
#include "Building/Building.h"
#include "Building/ResourceBuilding.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardSubsystem, Log, All );

UCardSubsystem* UCardSubsystem::Get( const UObject* worldContextObject )
{
	if ( !worldContextObject )
	{
		return nullptr;
	}

	UWorld* world = GEngine->GetWorldFromContextObject(
		worldContextObject, EGetWorldErrorMode::LogAndReturnNull
	);
	if ( !world )
	{
		return nullptr;
	}

	UGameInstance* gameInstance = world->GetGameInstance();
	if ( !gameInstance )
	{
		return nullptr;
	}

	return gameInstance->GetSubsystem<UCardSubsystem>();
}

void UCardSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );
	EconomyBonuses_.Reset();
	UE_LOG( LogCardSubsystem, Log, TEXT( "CardSubsystem initialized" ) );
}

void UCardSubsystem::Deinitialize()
{
	UnbindFromGameLoop();
	ResetCardHistory();
	PoolConfig_ = nullptr;
	UE_LOG( LogCardSubsystem, Log, TEXT( "CardSubsystem deinitialized" ) );
	Super::Deinitialize();
}

void UCardSubsystem::SetPoolConfig( UCardPoolConfig* config )
{
	PoolConfig_ = config;

	if ( !PoolConfig_ )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "SetPoolConfig: null config provided" ) );
		return;
	}

	UE_LOG( LogCardSubsystem, Log, TEXT( "Pool config set: %d cards, offer %d, select %d" ),
		PoolConfig_->CardPool.Num(),
		PoolConfig_->CardsToOffer,
		PoolConfig_->CardsToSelect
	);

	for ( UCardDataAsset* card : PoolConfig_->StartingCards )
	{
		if ( card )
		{
			ApplySingleCard( card, 0 );
		}
	}

	BindToGameLoop();
}


void UCardSubsystem::RequestCardSelection( int32 waveNumber )
{
	if ( !PoolConfig_ )
	{
		UE_LOG( LogCardSubsystem, Error,
			TEXT( "RequestCardSelection: No pool config set!" ) );
		NotifyGameLoopToProceed();
		return;
	}

	CurrentWaveNumber_ = waveNumber;

	TArray<UCardDataAsset*> availableCards = GenerateCardSelection( PoolConfig_->CardsToOffer );

	if ( availableCards.Num() == 0 )
	{
		UE_LOG( LogCardSubsystem, Warning,
			TEXT( "RequestCardSelection: No cards available for selection" ) );
		NotifyGameLoopToProceed();
		return;
	}

	FCardChoice choice;
	for ( UCardDataAsset* card : availableCards )
	{
		choice.AvailableCards.Add( card );
	}
	choice.CardsToSelect = PoolConfig_->CardsToSelect;
	choice.WaveNumber = waveNumber;

	UE_LOG( LogCardSubsystem, Log,
		TEXT( "Card selection requested: %d cards, select %d, wave %d" ),
		choice.AvailableCards.Num(), choice.CardsToSelect, choice.WaveNumber
	);

	OnCardSelectionRequired.Broadcast( choice );
}

void UCardSubsystem::ApplySelectedCards( const TArray<UCardDataAsset*>& selectedCards )
{
	if ( PoolConfig_ && selectedCards.Num() > PoolConfig_->CardsToSelect )
	{
		UE_LOG( LogCardSubsystem, Warning,
			TEXT( "ApplySelectedCards: Received %d cards but max allowed is %d — clamping" ),
			selectedCards.Num(), PoolConfig_->CardsToSelect
		);
	}

	const int32 maxToApply = PoolConfig_
		? FMath::Min( selectedCards.Num(), PoolConfig_->CardsToSelect )
		: selectedCards.Num();

	TArray<ABuilding*> cachedBuildings = GetAllBuildings();

	TArray<UCardDataAsset*> appliedList;

	for ( int32 i = 0; i < maxToApply; ++i )
	{
		UCardDataAsset* card = selectedCards[i];
		if ( card )
		{
			ApplySingleCardInternal( card, CurrentWaveNumber_, cachedBuildings );
			appliedList.Add( card );
		}
	}

	OnCardsApplied.Broadcast( appliedList );

	UE_LOG( LogCardSubsystem, Log, TEXT( "Applied %d selected cards" ), appliedList.Num() );

	NotifyGameLoopToProceed();
}

void UCardSubsystem::ApplySingleCard( UCardDataAsset* card, int32 waveNumber )
{
	if ( !card )
	{
		return;
	}

	TArray<ABuilding*> buildings = GetAllBuildings();
	ApplySingleCardInternal( card, waveNumber, buildings );
}

void UCardSubsystem::ApplySingleCardInternal(
	UCardDataAsset* card, int32 waveNumber, const TArray<ABuilding*>& buildings )
{
	if ( !card )
	{
		return;
	}

	if ( !card->bCanStack && GetCardStackCount( card ) > 0 )
	{
		UE_LOG( LogCardSubsystem, Warning,
			TEXT( "ApplySingleCard: Card '%s' cannot stack and is already applied — skipping" ),
			*card->CardName.ToString()
		);
		return;
	}

	const bool bIsGlobal = ( card->TargetFilter.BuildingType == EBuildingType::Any );

	bool bHasBuildingTargetedMods =
		card->HasBuildingModifiers() ||
		( card->HasResourceModifiers() && !bIsGlobal );

	if ( bHasBuildingTargetedMods )
	{
		int32 affectedCount = 0;

		for ( ABuilding* building : buildings )
		{
			if ( card->AppliesToBuilding( building ) )
			{
				ApplyCardToBuilding( card, building );
				affectedCount++;
			}
		}

		UE_LOG( LogCardSubsystem, Log,
			TEXT( "Applied card '%s' to %d buildings" ),
			*card->CardName.ToString(), affectedCount
		);
	}

	if ( card->HasResourceModifiers() && bIsGlobal )
	{
		ApplyGlobalEconomyModifiers( card, 1 );

		UE_LOG( LogCardSubsystem, Log,
			TEXT( "Applied card '%s' as global economy modifier" ),
			*card->CardName.ToString()
		);
	}

	bool bFoundExisting = false;
	for ( FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( record.Card == card )
		{
			record.StackCount++;
			bFoundExisting = true;
			break;
		}
	}

	if ( !bFoundExisting )
	{
		AppliedCardHistory_.Add( FAppliedCardRecord( card, waveNumber ) );
	}

	UE_LOG( LogCardSubsystem, Log, TEXT( "Card '%s' applied (wave %d)" ),
		*card->CardName.ToString(), waveNumber
	);
}

TArray<UCardDataAsset*> UCardSubsystem::GenerateCardSelection( int32 count )
{
	TArray<UCardDataAsset*> result;

	if ( !PoolConfig_ || PoolConfig_->CardPool.Num() == 0 )
	{
		return result;
	}

	TArray<TPair<UCardDataAsset*, float>> weightedCards;
	float totalWeight = 0.0f;

	for ( UCardDataAsset* card : PoolConfig_->CardPool )
	{
		if ( !card )
		{
			continue;
		}

		if ( !card->bCanStack )
		{
			bool bAlreadyApplied = false;
			for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
			{
				if ( record.Card == card )
				{
					bAlreadyApplied = true;
					break;
				}
			}

			if ( bAlreadyApplied )
			{
				continue;
			}
		}

		weightedCards.Add( TPair<UCardDataAsset*, float>( card, card->SelectionWeight ) );
		totalWeight += card->SelectionWeight;
	}

	while ( result.Num() < count && weightedCards.Num() > 0 )
	{
		float randomValue = FMath::FRand() * totalWeight;
		float currentWeight = 0.0f;
		bool bSelected = false;

		for ( int32 i = 0; i < weightedCards.Num(); ++i )
		{
			currentWeight += weightedCards[i].Value;

			if ( randomValue <= currentWeight )
			{
				result.Add( weightedCards[i].Key );
				totalWeight -= weightedCards[i].Value;
				weightedCards.RemoveAt( i );
				bSelected = true;
				break;
			}
		}

		if ( !bSelected && weightedCards.Num() > 0 )
		{
			int32 lastIndex = weightedCards.Num() - 1;
			result.Add( weightedCards[lastIndex].Key );
			totalWeight -= weightedCards[lastIndex].Value;
			weightedCards.RemoveAt( lastIndex );
		}
	}

	return result;
}

void UCardSubsystem::ApplyCardToBuilding( UCardDataAsset* card, ABuilding* building )
{
	if ( !card || !building )
	{
		return;
	}

	const bool bIsGlobal = ( card->TargetFilter.BuildingType == EBuildingType::Any );

	for ( const FCardStatModifier& modifier : card->Modifiers )
	{
		if ( !modifier.IsValid() )
		{
			continue;
		}

		if ( modifier.IsResourceModifier() && bIsGlobal )
		{
			continue;
		}

		ApplyModifierToBuilding( modifier, building );
	}
}

void UCardSubsystem::ApplyModifierToBuilding(
	const FCardStatModifier& modifier, ABuilding* building )
{
	if ( !building || !modifier.IsValid() )
	{
		return;
	}

	if ( modifier.Stat == EBuildingStat::MaintenanceCost )
	{
		ApplyMaintenanceCostModifier( modifier, building );
	}
	else if ( modifier.Stat == EBuildingStat::BuildingProduction )
	{
		ApplyProductionModifier( modifier, building );
	}
	else if ( modifier.IsBuildingStatModifier() )
	{
		ApplyEntityStatModifier( modifier, building );
	}
}

void UCardSubsystem::ApplyMaintenanceCostModifier(
	const FCardStatModifier& modifier, ABuilding* building )
{
	if ( !building )
	{
		return;
	}

	if ( modifier.ResourceTarget == EResourceTargetType::All )
	{
		building->ModifyMaintenanceCostAll( modifier.FlatValue );
	}
	else
	{
		EResourceType resourceType = CardTypeHelpers::ToResourceType( modifier.ResourceTarget );
		building->ModifyMaintenanceCost( resourceType, modifier.FlatValue );
	}

	UE_LOG( LogCardSubsystem, Verbose,
		TEXT( "Modified MaintenanceCost on %s by %d (%s)" ),
		*building->GetName(), modifier.FlatValue,
		*CardTypeHelpers::GetResourceName( modifier.ResourceTarget )
	);
}

void UCardSubsystem::ApplyProductionModifier(
	const FCardStatModifier& modifier, ABuilding* building )
{
	AResourceBuilding* resourceBuilding = Cast<AResourceBuilding>( building );
	if ( !resourceBuilding )
	{
		UE_LOG( LogCardSubsystem, Verbose,
			TEXT( "BuildingProduction modifier on non-ResourceBuilding %s — skipped" ),
			building ? *building->GetName() : TEXT( "null" )
		);
		return;
	}

	if ( modifier.ResourceTarget == EResourceTargetType::All )
	{
		resourceBuilding->ModifyProductionAll( modifier.FlatValue );
	}
	else
	{
		EResourceType resourceType = CardTypeHelpers::ToResourceType( modifier.ResourceTarget );
		resourceBuilding->ModifyProduction( resourceType, modifier.FlatValue );
	}

	UE_LOG( LogCardSubsystem, Verbose,
		TEXT( "Modified Production on %s by %d (%s)" ),
		*resourceBuilding->GetName(), modifier.FlatValue,
		*CardTypeHelpers::GetResourceName( modifier.ResourceTarget )
	);
}

void UCardSubsystem::ApplyEntityStatModifier(
	const FCardStatModifier& modifier, ABuilding* building )
{
	if ( !building || !modifier.IsBuildingStatModifier() )
	{
		return;
	}

	FEntityStats& stats = building->Stats();

	switch ( modifier.Stat )
	{
	case EBuildingStat::MaxHealth:
	{
		int32 newMaxHealth = stats.MaxHealth() + modifier.FlatValue;
		stats.SetMaxHealth( FMath::Max( 1, newMaxHealth ) );
		break;
	}

	case EBuildingStat::AttackDamage:
	{
		int32 newDamage = stats.AttackDamage() + modifier.FlatValue;
		stats.SetAttackDamage( FMath::Max( 0, newDamage ) );
		break;
	}

	case EBuildingStat::AttackRange:
	{
		float newRange = stats.AttackRange() + static_cast<float>( modifier.FlatValue );
		stats.SetAttackRange( FMath::Max( 0.0f, newRange ) );
		break;
	}

	case EBuildingStat::AttackCooldown:
	{
		// FlatValue is in "centiseconds": value of 10 = 0.1s change
		// Negative value = faster attacks (better)
		static constexpr float CooldownPerUnit = 0.01f;
		static constexpr float MinCooldown = 0.1f;

		float newCooldown = stats.AttackCooldown()
			+ static_cast<float>( modifier.FlatValue ) * CooldownPerUnit;
		stats.SetAttackCooldown( FMath::Max( MinCooldown, newCooldown ) );
		break;
	}

	case EBuildingStat::MaxSpeed:
	{
		float newSpeed = stats.MaxSpeed() + static_cast<float>( modifier.FlatValue );
		stats.SetMaxSpeed( FMath::Max( 0.0f, newSpeed ) );
		break;
	}

	default:
		break;
	}
}

// =============================================================================
// Economy
// =============================================================================

void UCardSubsystem::ApplyGlobalEconomyModifiers( UCardDataAsset* card, int32 stackCount )
{
	ApplyGlobalEconomyModifiersInternal( card, stackCount );
	OnEconomyBonusesChanged.Broadcast( EconomyBonuses_ );
}

void UCardSubsystem::ApplyGlobalEconomyModifiersInternal( UCardDataAsset* card, int32 stackCount )
{
	if ( !card )
	{
		return;
	}

	for ( const FCardStatModifier& modifier : card->Modifiers )
	{
		if ( modifier.IsResourceModifier() )
		{
			for ( int32 i = 0; i < stackCount; ++i )
			{
				EconomyBonuses_.ApplyModifier( modifier );
			}
		}
	}
}

void UCardSubsystem::RecalculateEconomyBonuses()
{
	EconomyBonuses_.Reset();

	for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( !record.Card || !record.Card->HasResourceModifiers() )
		{
			continue;
		}

		// Only global cards contribute to economy bonuses
		if ( record.Card->TargetFilter.BuildingType != EBuildingType::Any )
		{
			continue;
		}

		ApplyGlobalEconomyModifiersInternal( record.Card, record.StackCount );
	}

	// Single broadcast after full recalculation
	OnEconomyBonusesChanged.Broadcast( EconomyBonuses_ );
}

int32 UCardSubsystem::GetProductionBonus( EResourceTargetType resourceType ) const
{
	return EconomyBonuses_.GetProductionBonus( resourceType );
}

int32 UCardSubsystem::GetMaintenanceCostReduction( EResourceTargetType resourceType ) const
{
	return EconomyBonuses_.GetMaintenanceReduction( resourceType );
}

// =============================================================================
// Building Queries
// =============================================================================

TArray<ABuilding*> UCardSubsystem::GetAllBuildings() const
{
	TArray<ABuilding*> buildings;

	UWorld* world = GetWorldSafe();
	if ( !world )
	{
		return buildings;
	}

	for ( TActorIterator<ABuilding> it( world ); it; ++it )
	{
		ABuilding* building = *it;
		if ( building && !building->IsDestroyed() )
		{
			buildings.Add( building );
		}
	}

	return buildings;
}

int32 UCardSubsystem::GetCardStackCount( const UCardDataAsset* card ) const
{
	for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( record.Card == card )
		{
			return record.StackCount;
		}
	}
	return 0;
}

void UCardSubsystem::ResetCardHistory()
{
	// Revert building modifications BEFORE clearing history
	RevertAllBuildingModifiers();

	AppliedCardHistory_.Empty();
	EconomyBonuses_.Reset();
	CurrentWaveNumber_ = 0;

	OnEconomyBonusesChanged.Broadcast( EconomyBonuses_ );

	UE_LOG( LogCardSubsystem, Log, TEXT( "Card history reset — all modifiers reverted" ) );
}

void UCardSubsystem::RevertAllBuildingModifiers()
{
	TArray<ABuilding*> buildings = GetAllBuildings();

	for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( !record.Card )
		{
			continue;
		}

		const bool bIsGlobal = ( record.Card->TargetFilter.BuildingType == EBuildingType::Any );

		bool bHasTargetedModifiers =
			record.Card->HasBuildingModifiers() ||
			( record.Card->HasResourceModifiers() && !bIsGlobal );

		if ( !bHasTargetedModifiers )
		{
			continue;
		}

		for ( ABuilding* building : buildings )
		{
			if ( !record.Card->AppliesToBuilding( building ) )
			{
				continue;
			}

			// Apply inverse modifiers for each stack
			for ( int32 stack = 0; stack < record.StackCount; ++stack )
			{
				for ( const FCardStatModifier& modifier : record.Card->Modifiers )
				{
					if ( !modifier.IsValid() )
					{
						continue;
					}

					// Skip resource modifiers on global cards (economy, not building)
					if ( modifier.IsResourceModifier() && bIsGlobal )
					{
						continue;
					}

					// Create inverted modifier
					FCardStatModifier inverseMod = modifier;
					inverseMod.FlatValue = -modifier.FlatValue;

					ApplyModifierToBuilding( inverseMod, building );
				}
			}
		}
	}

	UE_LOG( LogCardSubsystem, Log,
		TEXT( "Reverted building modifiers from %d card records" ),
		AppliedCardHistory_.Num()
	);
}

TArray<FAppliedCardBonus> UCardSubsystem::GetBuildingBonuses( const ABuilding* building ) const
{
	TArray<FAppliedCardBonus> bonuses;

	if ( !building )
	{
		return bonuses;
	}

	for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( !record.Card || !record.Card->AppliesToBuilding( building ) )
		{
			continue;
		}

		for ( const FCardStatModifier& modifier : record.Card->Modifiers )
		{
			if ( !modifier.IsValid() )
			{
				continue;
			}

			// Skip resource modifiers on global cards (they're economy, not per-building)
			const bool bIsGlobal = ( record.Card->TargetFilter.BuildingType == EBuildingType::Any );
			if ( modifier.IsResourceModifier() && bIsGlobal )
			{
				continue;
			}

			FCardStatModifier totalMod = modifier;
			totalMod.FlatValue *= record.StackCount;

			bonuses.Add( FAppliedCardBonus(
				record.Card, totalMod, record.WaveSelected
			) );
		}
	}

	return bonuses;
}

void UCardSubsystem::OnBuildingPlaced( ABuilding* building )
{
	if ( !building )
	{
		return;
	}

	// Apply all existing cards to newly placed building
	for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( !record.Card )
		{
			continue;
		}

		// Check if card has modifiers that target this building
		const bool bIsGlobal = ( record.Card->TargetFilter.BuildingType == EBuildingType::Any );
		bool bHasTargetedModifiers =
			record.Card->HasBuildingModifiers() ||
			( record.Card->HasResourceModifiers() && !bIsGlobal );

		if ( bHasTargetedModifiers && record.Card->AppliesToBuilding( building ) )
		{
			for ( int32 i = 0; i < record.StackCount; ++i )
			{
				ApplyCardToBuilding( record.Card, building );
			}
		}
	}

	UE_LOG( LogCardSubsystem, Log,
		TEXT( "Applied existing cards to new building: %s" ), *building->GetName()
	);
}

// =============================================================================
// GameLoop Integration
// =============================================================================

void UCardSubsystem::BindToGameLoop()
{
	if ( bIsBoundToGameLoop_ )
	{
		return;
	}

	UCoreManager* coreManager = UCoreManager::Get( GetGameInstance() );
	if ( !coreManager )
	{
		return;
	}

	UGameLoopManager* gameLoop = coreManager->GetGameLoop();
	if ( !gameLoop )
	{
		return;
	}

	gameLoop->OnPhaseChanged.AddDynamic( this, &UCardSubsystem::HandlePhaseChanged );
	CachedGameLoop_ = gameLoop;
	bIsBoundToGameLoop_ = true;

	UE_LOG( LogCardSubsystem, Log, TEXT( "Bound to GameLoopManager" ) );
}

void UCardSubsystem::UnbindFromGameLoop()
{
	if ( !bIsBoundToGameLoop_ )
	{
		return;
	}

	if ( UGameLoopManager* gameLoop = CachedGameLoop_.Get() )
	{
		gameLoop->OnPhaseChanged.RemoveDynamic(
			this, &UCardSubsystem::HandlePhaseChanged
		);
	}

	CachedGameLoop_.Reset();
	bIsBoundToGameLoop_ = false;

	UE_LOG( LogCardSubsystem, Log, TEXT( "Unbound from GameLoopManager" ) );
}

void UCardSubsystem::HandlePhaseChanged(
	EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	if ( newPhase == EGameLoopPhase::Reward )
	{
		if ( UGameLoopManager* gameLoop = CachedGameLoop_.Get() )
		{
			int32 waveNumber = gameLoop->GetCurrentWave();
			RequestCardSelection( waveNumber );
		}
	}
}

void UCardSubsystem::NotifyGameLoopToProceed()
{
	if ( UGameLoopManager* gameLoop = CachedGameLoop_.Get() )
	{
		gameLoop->ConfirmRewardPhase();
	}
	else
	{
		UCoreManager* coreManager = UCoreManager::Get( GetGameInstance() );
		if ( coreManager )
		{
			if ( UGameLoopManager* loop = coreManager->GetGameLoop() )
			{
				loop->ConfirmRewardPhase();
			}
		}
	}
}

UWorld* UCardSubsystem::GetWorldSafe() const
{
	UGameInstance* gi = GetGameInstance();
	return gi ? gi->GetWorld() : nullptr;
}
