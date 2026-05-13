#include "Cards/CardSubsystem.h"

#include "Building/Building.h"
#include "Cards/CardCondition.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffect.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/CardPoolConfig.h"
#include "Cards/CardPoolResolver.h"
#include "Cards/CardRarityPoolConfig.h"
#include "Cards/Visuals/CardVisualSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Resources/ResourceManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardSubsystem, Log, All );

UCardSubsystem* UCardSubsystem::Get( const UObject* worldContextObject )
{
	if ( !worldContextObject )
	{
		return nullptr;
	}

	UWorld* world = GEngine->GetWorldFromContextObject( worldContextObject, EGetWorldErrorMode::LogAndReturnNull );
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
	if ( AppliedCardHistory_.Num() > 0 )
	{
		UE_LOG( LogCardSubsystem, Log, TEXT( "SetPoolConfig: resetting previous match history (%d records)" ),
			AppliedCardHistory_.Num() );
		ResetCardHistory();
	}

	PoolConfig_ = config;

	if ( !PoolConfig_ )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "SetPoolConfig: null config provided" ) );
		return;
	}

	UE_LOG(
	    LogCardSubsystem, Log, TEXT( "Pool config set: %d cards across %d rarity tiers, offer %d, select %d" ),
	    PoolConfig_->GetPoolSize(), PoolConfig_->RarityPools.Num(),
	    PoolConfig_->CardsToOffer, PoolConfig_->CardsToSelect
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
		UE_LOG( LogCardSubsystem, Error, TEXT( "RequestCardSelection: No pool config set!" ) );
		NotifyGameLoopToProceed();
		return;
	}

	ClearCurrentSelectionExclusions();

	FCardChoice choice;
	if ( !BuildCardChoice( waveNumber, choice ) )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "RequestCardSelection: No cards available for selection" ) );
		NotifyGameLoopToProceed();
		return;
	}

	UE_LOG(
	    LogCardSubsystem, Log, TEXT( "Card selection requested: %d cards, select %d, wave %d (debug=%d)" ),
	    choice.AvailableCards.Num(), choice.CardsToSelect, choice.WaveNumber,
	    PoolConfig_->bDebugShowAllCards ? 1 : 0
	);

	OnCardSelectionRequired.Broadcast( choice );
}

bool UCardSubsystem::BuildCardChoice( int32 waveNumber, FCardChoice& outChoice )
{
	if ( !PoolConfig_ )
	{
		return false;
	}

	CurrentWaveNumber_ = waveNumber;

	TArray<UCardDataAsset*> availableCards;

	if ( PoolConfig_->bDebugShowAllCards )
	{
		TSet<UCardDataAsset*> seen;
		for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : PoolConfig_->RarityPools )
		{
			if ( !rarityPool )
			{
				continue;
			}
			for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
			{
				UCardDataAsset* raw = card.Get();
				if ( raw && !seen.Contains( raw ) )
				{
					seen.Add( raw );
					availableCards.Add( raw );
				}
			}
		}
	}
	else
	{
		TArray<FCardRarityBucket> buckets;
		buckets.Reserve( PoolConfig_->RarityPools.Num() );

		for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : PoolConfig_->RarityPools )
		{
			if ( !rarityPool || rarityPool->RarityWeight <= 0.f )
			{
				continue;
			}

			const float waveMultiplier =
				PoolConfig_->GetRarityWeightMultiplierForWave( waveNumber, rarityPool->Rarity );

			FCardRarityBucket bucket;
			bucket.Rarity       = rarityPool->Rarity;
			bucket.RarityWeight = rarityPool->RarityWeight * waveMultiplier;
			bucket.Cards.Reserve( rarityPool->Cards.Num() );

			for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
			{
				if ( card && IsCardUnlocked( card.Get() ) )
				{
					bucket.Cards.Add( card.Get() );
				}
			}

			if ( bucket.Cards.Num() > 0 )
			{
				buckets.Add( MoveTemp( bucket ) );
			}
		}

		TSet<UCardDataAsset*> seenForWeightReduction;
		seenForWeightReduction.Reserve( CurrentSelectionExcluded_.Num() );
		for ( const TObjectPtr<UCardDataAsset>& card : CurrentSelectionExcluded_ )
		{
			if ( card )
			{
				seenForWeightReduction.Add( card.Get() );
			}
		}

		availableCards = FCardPoolResolver::Resolve(
			buckets,
			AppliedCardHistory_,
			PoolConfig_->CardsToOffer,
			PoolConfig_->MaxCardsPerRarityInOffering,
			PoolConfig_->MaxStacksForWeightInfluence,
			seenForWeightReduction,
			PoolConfig_->RerollSeenWeightMultiplier );
	}

	if ( availableCards.Num() == 0 )
	{
		return false;
	}

	for ( UCardDataAsset* card : availableCards )
	{
		if ( card )
		{
			CurrentSelectionExcluded_.Add( card );
		}
	}

	outChoice.AvailableCards.Reset( availableCards.Num() );
	for ( UCardDataAsset* card : availableCards )
	{
		outChoice.AvailableCards.Add( card );
	}
	outChoice.CardsToSelect = PoolConfig_->CardsToSelect;
	outChoice.WaveNumber = waveNumber;

	return true;
}

void UCardSubsystem::ClearCurrentSelectionExclusions()
{
	CurrentSelectionExcluded_.Reset();
}

bool UCardSubsystem::IsRerollEnabled() const
{
	return PoolConfig_ && PoolConfig_->bAllowReroll;
}

int32 UCardSubsystem::GetRerollCost( int32 rerollIndex ) const
{
	if ( !PoolConfig_ )
	{
		return 0;
	}
	return PoolConfig_->GetRerollCost( rerollIndex );
}

bool UCardSubsystem::CanAffordReroll( int32 rerollIndex ) const
{
	if ( !IsRerollEnabled() )
	{
		return false;
	}

	UCoreManager* coreManager = UCoreManager::Get( GetGameInstance() );
	UResourceManager* resourceManager = coreManager ? coreManager->GetResourceManager() : nullptr;
	if ( !resourceManager )
	{
		return false;
	}

	const int32 cost = GetRerollCost( rerollIndex );
	if ( cost <= 0 )
	{
		return true;
	}

	return resourceManager->HasEnoughResource( CardTypeHelpers::ToResourceType( PoolConfig_->RerollResource ), cost );
}

bool UCardSubsystem::TryRerollCardChoice( int32 waveNumber, int32 rerollIndex, FCardChoice& outChoice )
{
	if ( !IsRerollEnabled() )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "TryRerollCardChoice: Reroll is not enabled in pool config" ) );
		return false;
	}

	if ( PoolConfig_->MaxRerollsPerSelection > 0 && rerollIndex >= PoolConfig_->MaxRerollsPerSelection )
	{
		UE_LOG(
		    LogCardSubsystem, Log,
		    TEXT( "TryRerollCardChoice: Max rerolls (%d) reached" ), PoolConfig_->MaxRerollsPerSelection );
		return false;
	}

	UCoreManager* coreManager = UCoreManager::Get( GetGameInstance() );
	UResourceManager* resourceManager = coreManager ? coreManager->GetResourceManager() : nullptr;
	if ( !resourceManager )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "TryRerollCardChoice: No ResourceManager available" ) );
		return false;
	}

	const int32 cost = GetRerollCost( rerollIndex );
	const EResourceType resourceType = CardTypeHelpers::ToResourceType( PoolConfig_->RerollResource );

	if ( cost > 0 && !resourceManager->HasEnoughResource( resourceType, cost ) )
	{
		UE_LOG(
		    LogCardSubsystem, Log,
		    TEXT( "TryRerollCardChoice: Not enough %s for reroll (cost=%d)" ),
		    *CardTypeHelpers::GetResourceName( PoolConfig_->RerollResource ), cost );
		return false;
	}

	FCardChoice tentative;
	if ( !BuildCardChoice( waveNumber, tentative ) )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "TryRerollCardChoice: Failed to build new card choice" ) );
		return false;
	}

	if ( cost > 0 && !resourceManager->TrySpendResource( resourceType, cost ) )
	{
		UE_LOG(
		    LogCardSubsystem, Warning,
		    TEXT( "TryRerollCardChoice: TrySpendResource failed unexpectedly after pre-check" ) );
		return false;
	}

	outChoice = tentative;

	UE_LOG(
	    LogCardSubsystem, Log,
	    TEXT( "Cards rerolled: %d cards offered, paid %d %s (rerollIndex=%d)" ),
	    outChoice.AvailableCards.Num(), cost,
	    *CardTypeHelpers::GetResourceName( PoolConfig_->RerollResource ), rerollIndex );

	return true;
}

void UCardSubsystem::ApplySelectedCards( const TArray<UCardDataAsset*>& selectedCards )
{
	if ( PoolConfig_ && selectedCards.Num() > PoolConfig_->CardsToSelect )
	{
		UE_LOG(
		    LogCardSubsystem, Warning,
		    TEXT( "ApplySelectedCards: Received %d cards but max allowed is %d — clamping" ),
		    selectedCards.Num(), PoolConfig_->CardsToSelect
		);
	}

	const int32 maxToApply =
	    PoolConfig_ ? FMath::Min( selectedCards.Num(), PoolConfig_->CardsToSelect ) : selectedCards.Num();

	TArray<ABuilding*> cachedBuildings = GetAllBuildings();

	TArray<UCardDataAsset*> appliedList;
	appliedList.Reserve( maxToApply );

	for ( int32 i = 0; i < maxToApply; ++i )
	{
		UCardDataAsset* card = selectedCards[i];
		if ( card )
		{
			ApplySingleCardOnce( card, CurrentWaveNumber_, cachedBuildings );
			appliedList.Add( card );
		}
	}

	ClearCurrentSelectionExclusions();

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
	ApplySingleCardOnce( card, waveNumber, buildings );

	TArray<UCardDataAsset*> appliedList;
	appliedList.Add( card );
	OnCardsApplied.Broadcast( appliedList );
}

void UCardSubsystem::ApplySingleCardOnce(
    UCardDataAsset* card, int32 waveNumber, const TArray<ABuilding*>& buildings )
{
	if ( !card )
	{
		return;
	}

	if ( !card->bCanStack && GetCardStackCount( card ) > 0 )
	{
		UE_LOG(
		    LogCardSubsystem, Warning,
		    TEXT( "ApplySingleCard: Card '%s' cannot stack and is already applied — skipping" ),
		    *card->CardName.ToString()
		);
		return;
	}

	int32 newStackCount = 1;
	bool bFoundExisting = false;
	for ( FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		if ( record.Card == card )
		{
			record.StackCount++;
			newStackCount = record.StackCount;
			bFoundExisting = true;
			break;
		}
	}

	if ( !bFoundExisting )
	{
		AppliedCardHistory_.Add( FAppliedCardRecord( card, waveNumber ) );
		newStackCount = 1;
	}

	AcquisitionLog_.Add( card );

	for ( int32 eventIndex = 0; eventIndex < card->Events.Num(); ++eventIndex )
	{
		ApplyCardEvent( card, eventIndex, card->Events[eventIndex], waveNumber, newStackCount, buildings );
	}

	UE_LOG(
	    LogCardSubsystem, Log, TEXT( "Card '%s' applied (wave %d, stack %d)" ),
	    *card->CardName.ToString(), waveNumber, newStackCount );
}

void UCardSubsystem::ApplyCardEvent(
    UCardDataAsset* card, int32 eventIndex, const FCardEvent& event,
    int32 waveNumber, int32 stackCount, const TArray<ABuilding*>& buildings )
{
	bool bHasGlobalEffect = false;
	bool bHasPerBuildingEffect = false;

	for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
	{
		if ( !effect )
		{
			continue;
		}
		if ( effect->IsGlobalEffect() )
		{
			bHasGlobalEffect = true;
		}
		else
		{
			bHasPerBuildingEffect = true;
		}
	}

	if ( bHasGlobalEffect )
	{
		FCardEffectContext ctx = MakeContext( card, eventIndex, stackCount, waveNumber, nullptr );
		if ( EvaluateConditions( event, ctx ) )
		{
			for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
			{
				if ( effect && effect->IsGlobalEffect() )
				{
					effect->Apply( ctx );
				}
			}
			OnEconomyBonusesChanged.Broadcast( EconomyBonuses_ );
		}
	}

	if ( bHasPerBuildingEffect )
	{
		bool bHasOneShotEffect = false;
		for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
		{
			if ( effect && !effect->IsGlobalEffect() && !effect->RequiresRuntimeRegistration() )
			{
				bHasOneShotEffect = true;
				break;
			}
		}

		int32 affected = 0;
		for ( ABuilding* building : buildings )
		{
			if ( !IsValid( building ) )
			{
				continue;
			}
			// Class-only gate: runtime effects must register regardless of current
			// state so a Destroyed-Only card lands its registration even while the
			// building is alive — the dispatch path re-checks state per trigger.
			if ( !event.MatchesBuildingClass( building ) )
			{
				continue;
			}

			const bool bStateMatches = event.MatchesBuildingState( building );

			FCardEffectContext ctx = MakeContext( card, eventIndex, stackCount, waveNumber, building );

			const bool bOneShotPassesConditions = bHasOneShotEffect && bStateMatches
				? EvaluateConditions( event, ctx )
				: false;

			UCardEffectHostComponent* host = nullptr;
			UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this );
			for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
			{
				if ( !effect || effect->IsGlobalEffect() )
				{
					continue;
				}

				if ( effect->RequiresRuntimeRegistration() )
				{
					if ( !host )
					{
						host = EnsureEffectHost( building );
						ctx.EffectHost = host;
					}
					if ( host )
					{
						host->RegisterEffect( card, eventIndex, effect );
					}
				}
				else if ( bOneShotPassesConditions )
				{
					effect->Apply( ctx );
					if ( visuals && !effect->HandlesOwnVisuals() )
					{
						visuals->PlayOneShot( effect->VisualConfig, building, nullptr );
					}
				}
			}

			affected++;
		}

		UE_LOG(
		    LogCardSubsystem, Verbose, TEXT( "Event %d on '%s' affected %d buildings" ),
		    eventIndex, *card->CardName.ToString(), affected );
	}
}

FCardEffectContext UCardSubsystem::MakeContext(
    UCardDataAsset* card, int32 eventIndex, int32 stackCount, int32 waveNumber, ABuilding* building )
{
	FCardEffectContext ctx;
	ctx.SourceCard = card;
	ctx.Building = building;
	ctx.EventIndex = eventIndex;
	ctx.WaveNumber = waveNumber;
	ctx.StackCount = stackCount;
	ctx.Subsystem = this;

	if ( building )
	{
		ctx.EffectHost = building->FindComponentByClass<UCardEffectHostComponent>();
	}

	return ctx;
}

bool UCardSubsystem::EvaluateConditions( const FCardEvent& event, const FCardEffectContext& context ) const
{
	for ( const TObjectPtr<UCardCondition>& condition : event.Conditions )
	{
		if ( !condition )
		{
			continue;
		}
		if ( !condition->IsMet( context ) )
		{
			return false;
		}
	}
	return true;
}

int32 UCardSubsystem::GetProductionBonus( EResourceTargetType resourceType ) const
{
	return EconomyBonuses_.GetProductionBonus( resourceType );
}

int32 UCardSubsystem::GetMaintenanceCostReduction( EResourceTargetType resourceType ) const
{
	return EconomyBonuses_.GetMaintenanceReduction( resourceType );
}

void UCardSubsystem::AddEconomyProductionBonus( EResourceTargetType target, int32 delta )
{
	EconomyBonuses_.AddProductionBonus( target, delta );
}

void UCardSubsystem::AddEconomyMaintenanceReduction( EResourceTargetType target, int32 delta )
{
	EconomyBonuses_.AddMaintenanceReduction( target, delta );
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

bool UCardSubsystem::IsCardUnlocked( const UCardDataAsset* card ) const
{
	if ( !card )
	{
		return false;
	}

	UCardDataAsset* mutableCard = const_cast<UCardDataAsset*>( card );

	if ( RuntimeLocked_.Contains( mutableCard ) )
	{
		return false;
	}

	if ( RuntimeUnlocked_.Contains( mutableCard ) )
	{
		return true;
	}

	if ( card->bUnlockedByDefault )
	{
		return true;
	}

	for ( const TObjectPtr<UCardDataAsset>& unlocker : card->UnlockedBy )
	{
		if ( !unlocker || unlocker == card )
		{
			continue;
		}
		for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
		{
			if ( record.Card == unlocker )
			{
				return true;
			}
		}
	}

	return false;
}

void UCardSubsystem::UnlockCard( UCardDataAsset* card )
{
	if ( !card )
	{
		return;
	}

	RuntimeLocked_.Remove( card );
	if ( !card->bUnlockedByDefault )
	{
		RuntimeUnlocked_.Add( card );
	}

	UE_LOG( LogCardSubsystem, Log, TEXT( "Card '%s' unlocked" ), *card->CardName.ToString() );
}

void UCardSubsystem::LockCard( UCardDataAsset* card )
{
	if ( !card )
	{
		return;
	}

	RuntimeUnlocked_.Remove( card );
	if ( card->bUnlockedByDefault )
	{
		RuntimeLocked_.Add( card );
	}

	UE_LOG( LogCardSubsystem, Log, TEXT( "Card '%s' locked" ), *card->CardName.ToString() );
}

void UCardSubsystem::UnlockCardByID( FName cardID )
{
	if ( !PoolConfig_ || cardID.IsNone() )
	{
		return;
	}

	for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : PoolConfig_->RarityPools )
	{
		if ( !rarityPool )
		{
			continue;
		}
		for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
		{
			if ( card && card->CardID == cardID )
			{
				UnlockCard( card.Get() );
				return;
			}
		}
	}

	UE_LOG( LogCardSubsystem, Warning, TEXT( "UnlockCardByID: '%s' not found in pool" ), *cardID.ToString() );
}

void UCardSubsystem::LockCardByID( FName cardID )
{
	if ( !PoolConfig_ || cardID.IsNone() )
	{
		return;
	}

	for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : PoolConfig_->RarityPools )
	{
		if ( !rarityPool )
		{
			continue;
		}
		for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
		{
			if ( card && card->CardID == cardID )
			{
				LockCard( card.Get() );
				return;
			}
		}
	}

	UE_LOG( LogCardSubsystem, Warning, TEXT( "LockCardByID: '%s' not found in pool" ), *cardID.ToString() );
}

void UCardSubsystem::ResetUnlocksToDefaults()
{
	RuntimeUnlocked_.Empty();
	RuntimeLocked_.Empty();
	UE_LOG( LogCardSubsystem, Log, TEXT( "Card unlocks reset to defaults" ) );
}

TArray<UCardDataAsset*> UCardSubsystem::GetUnlockedCards() const
{
	TArray<UCardDataAsset*> result;
	if ( !PoolConfig_ )
	{
		return result;
	}

	result.Reserve( PoolConfig_->GetPoolSize() );
	for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : PoolConfig_->RarityPools )
	{
		if ( !rarityPool )
		{
			continue;
		}
		for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
		{
			if ( card && IsCardUnlocked( card.Get() ) )
			{
				result.Add( card.Get() );
			}
		}
	}
	return result;
}

void UCardSubsystem::ResetCardHistory()
{
	for ( int32 i = AppliedCardHistory_.Num() - 1; i >= 0; --i )
	{
		RevertAppliedRecord( AppliedCardHistory_[i] );
	}

	AppliedCardHistory_.Empty();
	AcquisitionLog_.Empty();
	EconomyBonuses_.Reset();
	CurrentWaveNumber_ = 0;

	UWorld* world = GetWorldSafe();
	if ( world )
	{
		for ( TActorIterator<ABuilding> it( world ); it; ++it )
		{
			if ( ABuilding* b = *it )
			{
				if ( UCardEffectHostComponent* host = b->FindComponentByClass<UCardEffectHostComponent>() )
				{
					host->ClearAll();
				}
			}
		}
	}

	OnEconomyBonusesChanged.Broadcast( EconomyBonuses_ );

	UE_LOG( LogCardSubsystem, Log, TEXT( "Card history reset — all modifiers reverted" ) );
}

void UCardSubsystem::RevertAppliedRecord( const FAppliedCardRecord& record )
{
	UCardDataAsset* card = record.Card;
	if ( !card )
	{
		return;
	}

	TArray<ABuilding*> buildings = GetAllBuildings();

	for ( int32 stackStep = record.StackCount; stackStep >= 1; --stackStep )
	{
		for ( int32 eventIndex = 0; eventIndex < card->Events.Num(); ++eventIndex )
		{
			const FCardEvent& event = card->Events[eventIndex];

			bool bHasGlobal = false;
			bool bHasPerBuilding = false;
			for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
			{
				if ( !effect )
				{
					continue;
				}
				if ( effect->IsGlobalEffect() )
				{
					bHasGlobal = true;
				}
				else
				{
					bHasPerBuilding = true;
				}
			}

			if ( bHasGlobal )
			{
				FCardEffectContext ctx = MakeContext( card, eventIndex, stackStep, record.WaveSelected, nullptr );
				for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
				{
					if ( effect && effect->IsGlobalEffect() )
					{
						effect->Revert( ctx );
					}
				}
			}

			if ( bHasPerBuilding )
			{
				for ( ABuilding* building : buildings )
				{
					// Revert covers anything we applied — apply gate is class-only
					// (state may change between apply and revert), so use the same
					// gate here to undo cleanly.
					if ( !IsValid( building ) || !event.MatchesBuildingClass( building ) )
					{
						continue;
					}
					FCardEffectContext ctx = MakeContext( card, eventIndex, stackStep, record.WaveSelected, building );
					for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
					{
						if ( effect && !effect->IsGlobalEffect() )
						{
							effect->Revert( ctx );
						}
					}
				}
			}
		}
	}
}

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
		if ( IsValid( building ) )
		{
			buildings.Add( building );
		}
	}

	return buildings;
}

UCardEffectHostComponent* UCardSubsystem::EnsureEffectHost( ABuilding* building ) const
{
	if ( !building )
	{
		return nullptr;
	}

	if ( UCardEffectHostComponent* existing = building->FindComponentByClass<UCardEffectHostComponent>() )
	{
		return existing;
	}

	UCardEffectHostComponent* host = NewObject<UCardEffectHostComponent>( building );
	if ( host )
	{
		host->RegisterComponent();
	}
	return host;
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
		if ( !record.Card )
		{
			continue;
		}

		for ( const FCardEvent& event : record.Card->Events )
		{
			if ( !event.MatchesBuilding( building ) )
			{
				continue;
			}

			for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
			{
				if ( effect && !effect->IsGlobalEffect() )
				{
					bonuses.Add( FAppliedCardBonus( record.Card, effect, record.WaveSelected ) );
				}
			}
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

	for ( const FAppliedCardRecord& record : AppliedCardHistory_ )
	{
		UCardDataAsset* card = record.Card;
		if ( !card )
		{
			continue;
		}

		for ( int32 stack = 0; stack < record.StackCount; ++stack )
		{
			for ( int32 eventIndex = 0; eventIndex < card->Events.Num(); ++eventIndex )
			{
				const FCardEvent& event = card->Events[eventIndex];

				bool bHasOneShot = false;
				bool bHasRuntime = false;
				for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
				{
					if ( !effect || effect->IsGlobalEffect() )
					{
						continue;
					}
					if ( effect->RequiresRuntimeRegistration() )
					{
						bHasRuntime = true;
					}
					else
					{
						bHasOneShot = true;
					}
				}
				if ( !bHasOneShot && !bHasRuntime )
				{
					continue;
				}

				if ( !event.MatchesBuildingClass( building ) )
				{
					continue;
				}

				const bool bStateMatches = event.MatchesBuildingState( building );

				FCardEffectContext ctx = MakeContext( card, eventIndex, stack + 1, record.WaveSelected, building );

				const bool bOneShotPassesConditions = bHasOneShot && bStateMatches
					? EvaluateConditions( event, ctx )
					: false;

				UCardEffectHostComponent* host = nullptr;
				for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
				{
					if ( !effect || effect->IsGlobalEffect() )
					{
						continue;
					}

					if ( effect->RequiresRuntimeRegistration() )
					{
						if ( !host )
						{
							host = EnsureEffectHost( building );
							ctx.EffectHost = host;
						}
						if ( host )
						{
							host->RegisterEffect( card, eventIndex, effect );
						}
					}
					else if ( bOneShotPassesConditions )
					{
						effect->Apply( ctx );
					}
				}
			}
		}
	}

	UE_LOG( LogCardSubsystem, Log, TEXT( "Applied existing cards to new building: %s" ), *building->GetName() );
}

void UCardSubsystem::DebugApplyCardByID( FName cardID )
{
	if ( !PoolConfig_ || cardID.IsNone() )
	{
		return;
	}

	for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : PoolConfig_->RarityPools )
	{
		if ( !rarityPool )
		{
			continue;
		}
		for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
		{
			if ( card && card->CardID == cardID )
			{
				ApplySingleCard( card.Get(), CurrentWaveNumber_ );
				UE_LOG( LogCardSubsystem, Log, TEXT( "Debug: applied card '%s'" ), *cardID.ToString() );
				return;
			}
		}
	}

	UE_LOG( LogCardSubsystem, Warning, TEXT( "Debug: card '%s' not found in pool" ), *cardID.ToString() );
}

void UCardSubsystem::ToggleDebugShowAll()
{
	if ( !PoolConfig_ )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "ToggleDebugShowAll: no pool config set" ) );
		return;
	}

	PoolConfig_->bDebugShowAllCards = !PoolConfig_->bDebugShowAllCards;
	UE_LOG(
	    LogCardSubsystem, Log, TEXT( "Debug ShowAllCards = %d" ),
	    PoolConfig_->bDebugShowAllCards ? 1 : 0 );
}

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
		gameLoop->OnPhaseChanged.RemoveDynamic( this, &UCardSubsystem::HandlePhaseChanged );
	}

	CachedGameLoop_.Reset();
	bIsBoundToGameLoop_ = false;

	UE_LOG( LogCardSubsystem, Log, TEXT( "Unbound from GameLoopManager" ) );
}

void UCardSubsystem::HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	if ( newPhase == EGameLoopPhase::Reward )
	{
		if ( UGameLoopManager* gameLoop = CachedGameLoop_.Get() )
		{
			if ( gameLoop->IsLastWave() )
			{
				return;
			}

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
		return;
	}

	UCoreManager* coreManager = UCoreManager::Get( GetGameInstance() );
	if ( coreManager )
	{
		if ( UGameLoopManager* loop = coreManager->GetGameLoop() )
		{
			loop->ConfirmRewardPhase();
		}
	}
}

UWorld* UCardSubsystem::GetWorldSafe() const
{
	UGameInstance* gi = GetGameInstance();
	return gi ? gi->GetWorld() : nullptr;
}
