#include "Cards/CardEffectHostComponent.h"

#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Cards/CardCondition.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffect.h"
#include "Cards/CardSubsystem.h"
#include "Cards/Visuals/CardVisualSubsystem.h"
#include "Components/Attack/AttackRangedComponent.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Entity.h"
#include "EntityStats.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardEffectHost, Log, All );

namespace
{
	bool HasStickyVisual( const UCardEffect* effect )
	{
		if ( !effect )
		{
			return false;
		}

		const FCardVisualConfig& cfg = effect->VisualConfig;

		const bool bLoopingNiagara = !cfg.Niagara.System.IsNull()
			&& cfg.Niagara.bLoop
			&& cfg.Niagara.SpawnOn != ECardVisualTarget::None;

		const bool bStickyIcon = !cfg.Icon.Icon.IsNull()
			&& cfg.Icon.Mode == ECardVisualIconMode::Sticky
			&& cfg.Icon.ShowOn != ECardVisualTarget::None;

		return bLoopingNiagara || bStickyIcon;
	}
}

UCardEffectHostComponent::UCardEffectHostComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCardEffectHostComponent::BeginPlay()
{
	Super::BeginPlay();
	BindAttackDelegates();
	BindBuildingDelegates();
	BindDamageEvents();
	StartAuraTimer();
}

void UCardEffectHostComponent::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	ClearAll();
	Super::EndPlay( endPlayReason );
}

void UCardEffectHostComponent::RegisterEffect( UCardDataAsset* card, int32 eventIndex, UCardEffect* effect )
{
	if ( !card || !effect )
	{
		return;
	}

	FRegisteredCardEffect record;
	record.SourceCard = card;
	record.EventIndex = eventIndex;
	record.Effect = effect;

	if ( HasStickyVisual( effect ) )
	{
		if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
		{
			record.StickyHandle = visuals->BeginSticky( effect->VisualConfig, GetOwner(), nullptr );
		}
	}

	Active_.Add( record );

	BindAttackDelegates();
	BindBuildingDelegates();
	BindDamageEvents();
	StartAuraTimer();
}

void UCardEffectHostComponent::UnregisterBySourceCard( UCardDataAsset* card )
{
	if ( !card )
	{
		return;
	}

	UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this );

	for ( int32 i = Active_.Num() - 1; i >= 0; --i )
	{
		if ( Active_[i].SourceCard != card )
		{
			continue;
		}
		if ( visuals && Active_[i].StickyHandle.IsValid() )
		{
			visuals->EndSticky( Active_[i].StickyHandle );
		}
		Active_.RemoveAt( i );
	}
}

void UCardEffectHostComponent::ClearAll()
{
	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		for ( FRegisteredCardEffect& rec : Active_ )
		{
			if ( rec.StickyHandle.IsValid() )
			{
				visuals->EndSticky( rec.StickyHandle );
			}
		}
	}

	Active_.Empty();
	Counters_.Empty();
	UnbindAttackDelegates();
	UnbindBuildingDelegates();
	UnbindDamageEvents();
	StopAuraTimer();
}

int32 UCardEffectHostComponent::GetCounter( FName key ) const
{
	if ( const int32* value = Counters_.Find( key ) )
	{
		return *value;
	}
	return 0;
}

void UCardEffectHostComponent::SetCounter( FName key, int32 value )
{
	Counters_.FindOrAdd( key ) = value;
}

int32 UCardEffectHostComponent::IncrementCounter( FName key )
{
	int32& slot = Counters_.FindOrAdd( key );
	slot += 1;
	return slot;
}

FName UCardEffectHostComponent::MakeCounterKey( const UCardDataAsset* card, int32 eventIndex, FName localTag )
{
	const FString cardName = card ? card->GetName() : TEXT( "null" );
	return FName( *FString::Printf( TEXT( "%s#%d#%s" ), *cardName, eventIndex, *localTag.ToString() ) );
}

void UCardEffectHostComponent::BindAttackDelegates()
{
	if ( bIsBoundToAttack_ )
	{
		return;
	}

	ADefensiveBuilding* tower = Cast<ADefensiveBuilding>( GetOwner() );
	if ( !tower )
	{
		return;
	}

	UAttackRangedComponent* attack = tower->FindComponentByClass<UAttackRangedComponent>();
	if ( !attack )
	{
		return;
	}

	attack->OnAttackFired.AddDynamic( this, &UCardEffectHostComponent::HandleAttackFired );
	attack->OnBeforeAttackFire.AddDynamic( this, &UCardEffectHostComponent::HandleBeforeAttackFire );
	attack->OnAttackTargetChanged.AddDynamic( this, &UCardEffectHostComponent::HandleAttackTargetChanged );

	BoundAttack_ = attack;
	bIsBoundToAttack_ = true;
}

void UCardEffectHostComponent::UnbindAttackDelegates()
{
	if ( !bIsBoundToAttack_ )
	{
		return;
	}

	if ( UAttackRangedComponent* attack = BoundAttack_.Get() )
	{
		attack->OnAttackFired.RemoveDynamic( this, &UCardEffectHostComponent::HandleAttackFired );
		attack->OnBeforeAttackFire.RemoveDynamic( this, &UCardEffectHostComponent::HandleBeforeAttackFire );
		attack->OnAttackTargetChanged.RemoveDynamic( this, &UCardEffectHostComponent::HandleAttackTargetChanged );
	}

	BoundAttack_.Reset();
	bIsBoundToAttack_ = false;
}

void UCardEffectHostComponent::HandleAttackFired( AActor* target )
{
	DispatchTrigger( ECardTriggerReason::AttackFired, target );
}

void UCardEffectHostComponent::HandleBeforeAttackFire( AActor* target )
{
	DispatchTrigger( ECardTriggerReason::BeforeAttackFire, target );
}

void UCardEffectHostComponent::HandleAttackTargetChanged( AActor* oldTarget, AActor* newTarget )
{
	DispatchTrigger( ECardTriggerReason::TargetChanged, newTarget );
}

void UCardEffectHostComponent::BindBuildingDelegates()
{
	if ( bIsBoundToBuilding_ )
	{
		return;
	}

	ABuilding* building = Cast<ABuilding>( GetOwner() );
	if ( !building )
	{
		return;
	}

	building->OnBuildingDamaged.AddDynamic( this, &UCardEffectHostComponent::HandleOwnerDamaged );
	building->OnBuildingDied.AddDynamic( this, &UCardEffectHostComponent::HandleOwnerRuined );
	bIsBoundToBuilding_ = true;
}

void UCardEffectHostComponent::UnbindBuildingDelegates()
{
	if ( !bIsBoundToBuilding_ )
	{
		return;
	}

	if ( ABuilding* building = Cast<ABuilding>( GetOwner() ) )
	{
		building->OnBuildingDamaged.RemoveDynamic( this, &UCardEffectHostComponent::HandleOwnerDamaged );
		building->OnBuildingDied.RemoveDynamic( this, &UCardEffectHostComponent::HandleOwnerRuined );
	}
	bIsBoundToBuilding_ = false;
}

void UCardEffectHostComponent::HandleOwnerDamaged( ABuilding* building, int32 damage, AActor* instigator )
{
	DispatchTrigger( ECardTriggerReason::Damaged, instigator, damage );
}

void UCardEffectHostComponent::HandleOwnerRuined( ABuilding* building )
{
	DispatchTrigger( ECardTriggerReason::Ruined, nullptr );
}

void UCardEffectHostComponent::BindDamageEvents()
{
	if ( bIsBoundToDamageEvents_ )
	{
		return;
	}

	DamageEventHandle_ = FDamageEvents::OnDamageDealt.AddUObject( this, &UCardEffectHostComponent::HandleDamageDealt );
	MissEventHandle_ = FDamageEvents::OnProjectileMissed.AddUObject( this, &UCardEffectHostComponent::HandleProjectileMissed );
	bIsBoundToDamageEvents_ = true;
}

void UCardEffectHostComponent::UnbindDamageEvents()
{
	if ( !bIsBoundToDamageEvents_ )
	{
		return;
	}

	FDamageEvents::OnDamageDealt.Remove( DamageEventHandle_ );
	FDamageEvents::OnProjectileMissed.Remove( MissEventHandle_ );
	DamageEventHandle_.Reset();
	MissEventHandle_.Reset();
	bIsBoundToDamageEvents_ = false;
}

void UCardEffectHostComponent::HandleProjectileMissed( AActor* instigator )
{
	if ( instigator != GetOwner() )
	{
		return;
	}
	DispatchTrigger( ECardTriggerReason::Missed, nullptr );
}

void UCardEffectHostComponent::StartAuraTimer()
{
	if ( bIsAuraTicking_ || AuraTickIntervalSeconds_ <= 0.f )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	world->GetTimerManager().SetTimer(
		AuraTimerHandle_, this, &UCardEffectHostComponent::HandleAuraTick,
		AuraTickIntervalSeconds_, true );
	bIsAuraTicking_ = true;
}

void UCardEffectHostComponent::StopAuraTimer()
{
	if ( !bIsAuraTicking_ )
	{
		return;
	}

	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( AuraTimerHandle_ );
	}
	bIsAuraTicking_ = false;
}

void UCardEffectHostComponent::HandleAuraTick()
{
	if ( Active_.Num() == 0 )
	{
		return;
	}
	DispatchTrigger( ECardTriggerReason::AuraTick, nullptr );
}

void UCardEffectHostComponent::HandleDamageDealt( AActor* instigator, AActor* target, int damage, bool bIsSplash )
{
	if ( instigator != GetOwner() )
	{
		return;
	}

	const IEntity* entity = Cast<IEntity>( target );
	const bool bWillBeKilled = entity
		&& entity->Stats().IsAlive()
		&& entity->Stats().Health() <= damage;

	UE_LOG( LogCardEffectHost, Verbose,
		TEXT( "[%s] HandleDamageDealt target=%s damage=%d entity=%d willKill=%d hp=%d" ),
		*GetNameSafe( GetOwner() ),
		*GetNameSafe( target ),
		damage,
		entity ? 1 : 0,
		bWillBeKilled ? 1 : 0,
		entity ? entity->Stats().Health() : -1 );

	DispatchTrigger( ECardTriggerReason::HitLanded, target, damage );

	if ( bWillBeKilled )
	{
		DispatchTrigger( ECardTriggerReason::KillLanded, target, damage );
	}
}

void UCardEffectHostComponent::DispatchTrigger( ECardTriggerReason reason, AActor* instigator, int32 magnitude )
{
	if ( Active_.Num() == 0 )
	{
		return;
	}

	UCardSubsystem* subsystem = UCardSubsystem::Get( this );
	UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this );
	ABuilding* building = Cast<ABuilding>( GetOwner() );

	UE_LOG( LogCardEffectHost, Verbose,
		TEXT( "[%s] DispatchTrigger reason=%d active=%d instigator=%s" ),
		*GetNameSafe( GetOwner() ),
		static_cast<int32>( reason ),
		Active_.Num(),
		*GetNameSafe( instigator ) );

	for ( const FRegisteredCardEffect& rec : Active_ )
	{
		if ( !rec.Effect || !rec.SourceCard )
		{
			continue;
		}

		FCardEffectContext ctx;
		ctx.SourceCard = rec.SourceCard;
		ctx.Building = building;
		ctx.EventIndex = rec.EventIndex;
		ctx.EffectHost = this;
		ctx.Subsystem = subsystem;
		ctx.EventInstigator = instigator;
		ctx.ActionMagnitude = magnitude;
		ctx.TriggerReason = reason;
		if ( instigator )
		{
			ctx.EventLocation = instigator->GetActorLocation();
			ctx.bHasEventLocation = true;
		}

		bool bConditionsPass = true;
		if ( rec.SourceCard->Events.IsValidIndex( rec.EventIndex ) )
		{
			const FCardEvent& event = rec.SourceCard->Events[rec.EventIndex];
			for ( const TObjectPtr<UCardCondition>& cond : event.Conditions )
			{
				if ( cond && !cond->IsMet( ctx ) )
				{
					bConditionsPass = false;
					UE_LOG( LogCardEffectHost, Verbose,
						TEXT( "  card=%s event=%d cond=%s FAIL" ),
						*GetNameSafe( rec.SourceCard ),
						rec.EventIndex,
						*GetNameSafe( cond ) );
					break;
				}
			}
		}
		if ( !bConditionsPass )
		{
			continue;
		}

		UE_LOG( LogCardEffectHost, Verbose,
			TEXT( "  card=%s event=%d effect=%s EXECUTE" ),
			*GetNameSafe( rec.SourceCard ),
			rec.EventIndex,
			*GetNameSafe( rec.Effect ) );

		rec.Effect->Execute( ctx );

		if ( visuals && building )
		{
			visuals->PlayOneShot( rec.Effect->VisualConfig, building, instigator );
		}
	}
}
