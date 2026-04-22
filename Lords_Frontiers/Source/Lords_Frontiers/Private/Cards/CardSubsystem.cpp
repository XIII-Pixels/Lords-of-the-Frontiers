#include "Cards/CardSubsystem.h"

#include "Building/Building.h"
#include "Cards/CardCondition.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffect.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/CardPoolConfig.h"
#include "Cards/CardPoolResolver.h"
#include "Cards/Feedback/CardFeedback.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"

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
	    LogCardSubsystem, Log, TEXT( "Pool config set: %d cards, offer %d, select %d" ),
	    PoolConfig_->CardPool.Num(), PoolConfig_->CardsToOffer, PoolConfig_->CardsToSelect
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

	CurrentWaveNumber_ = waveNumber;

	TArray<UCardDataAsset*> availableCards;

	if ( PoolConfig_->bDebugShowAllCards )
	{
		availableCards.Reserve( PoolConfig_->CardPool.Num() );
		for ( const TObjectPtr<UCardDataAsset>& card : PoolConfig_->CardPool )
		{
			if ( card )
			{
				availableCards.Add( card.Get() );
			}
		}
	}
	else
	{
		TArray<TObjectPtr<UCardDataAsset>> unlockedPool;
		unlockedPool.Reserve( PoolConfig_->CardPool.Num() );
		for ( const TObjectPtr<UCardDataAsset>& card : PoolConfig_->CardPool )
		{
			if ( card && IsCardUnlocked( card.Get() ) )
			{
				unlockedPool.Add( card );
			}
		}

		availableCards = FCardPoolResolver::Resolve(
			unlockedPool, AppliedCardHistory_, PoolConfig_->CardsToOffer );
	}

	if ( availableCards.Num() == 0 )
	{
		UE_LOG( LogCardSubsystem, Warning, TEXT( "RequestCardSelection: No cards available for selection" ) );
		NotifyGameLoopToProceed();
		return;
	}

	FCardChoice choice;
	choice.AvailableCards.Reserve( availableCards.Num() );
	for ( UCardDataAsset* card : availableCards )
	{
		choice.AvailableCards.Add( card );
	}
	choice.CardsToSelect = PoolConfig_->CardsToSelect;
	choice.WaveNumber = waveNumber;

	UE_LOG(
	    LogCardSubsystem, Log, TEXT( "Card selection requested: %d cards, select %d, wave %d (debug=%d)" ),
	    choice.AvailableCards.Num(), choice.CardsToSelect, choice.WaveNumber,
	    PoolConfig_->bDebugShowAllCards ? 1 : 0
	);

	OnCardSelectionRequired.Broadcast( choice );
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
			if ( !event.MatchesBuilding( building ) )
			{
				continue;
			}

			FCardEffectContext ctx = MakeContext( card, eventIndex, stackCount, waveNumber, building );

			const bool bOneShotPassesConditions = bHasOneShotEffect
				? EvaluateConditions( event, ctx )
				: false;

			UCardEffectHostComponent* host = nullptr;
			bool bAnyOneShotApplied = false;
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
					bAnyOneShotApplied = true;
				}
			}

			if ( bAnyOneShotApplied && card->bShowIconOnTrigger && card->FeedbackIconOverride )
			{
				UCardFeedback::ShowIconOnActor( this, building, card->FeedbackIconOverride );
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

	return card->bUnlockedByDefault;
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

	for ( const TObjectPtr<UCardDataAsset>& card : PoolConfig_->CardPool )
	{
		if ( card && card->CardID == cardID )
		{
			UnlockCard( card.Get() );
			return;
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

	for ( const TObjectPtr<UCardDataAsset>& card : PoolConfig_->CardPool )
	{
		if ( card && card->CardID == cardID )
		{
			LockCard( card.Get() );
			return;
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

	result.Reserve( PoolConfig_->CardPool.Num() );
	for ( const TObjectPtr<UCardDataAsset>& card : PoolConfig_->CardPool )
	{
		if ( card && IsCardUnlocked( card.Get() ) )
		{
			result.Add( card.Get() );
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
					if ( !IsValid( building ) || !event.MatchesBuilding( building ) )
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

				bool bHasPerBuilding = false;
				for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
				{
					if ( effect && !effect->IsGlobalEffect() )
					{
						bHasPerBuilding = true;
						break;
					}
				}
				if ( !bHasPerBuilding )
				{
					continue;
				}

				if ( !event.MatchesBuilding( building ) )
				{
					continue;
				}

				FCardEffectContext ctx = MakeContext( card, eventIndex, stack + 1, record.WaveSelected, building );
				if ( !EvaluateConditions( event, ctx ) )
				{
					continue;
				}

				for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
				{
					if ( !effect || effect->IsGlobalEffect() )
					{
						continue;
					}
					effect->Apply( ctx );

					if ( effect->RequiresRuntimeRegistration() )
					{
						if ( UCardEffectHostComponent* host = EnsureEffectHost( building ) )
						{
							host->RegisterEffect( card, eventIndex, effect );
						}
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

	for ( const TObjectPtr<UCardDataAsset>& card : PoolConfig_->CardPool )
	{
		if ( card && card->CardID == cardID )
		{
			ApplySingleCard( card.Get(), CurrentWaveNumber_ );
			UE_LOG( LogCardSubsystem, Log, TEXT( "Debug: applied card '%s'" ), *cardID.ToString() );
			return;
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
