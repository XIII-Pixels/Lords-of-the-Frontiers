#include "Cards/CardEffectHostComponent.h"

#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Cards/CardCondition.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffect.h"
#include "Cards/CardSubsystem.h"
#include "Components/Attack/AttackRangedComponent.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Entity.h"
#include "EntityStats.h"

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
	StopAuraTimer();
	UnbindAttackDelegates();
	UnbindBuildingDelegates();
	UnbindDamageEvents();
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

	Active_.RemoveAll( [card]( const FRegisteredCardEffect& r )
	{
		return r.SourceCard == card;
	} );
}

void UCardEffectHostComponent::ClearAll()
{
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
		attack->OnAttackTargetChanged.RemoveDynamic( this, &UCardEffectHostComponent::HandleAttackTargetChanged );
	}

	BoundAttack_.Reset();
	bIsBoundToAttack_ = false;
}

void UCardEffectHostComponent::HandleAttackFired( AActor* target )
{
	DispatchTrigger( ECardTriggerReason::AttackFired, target );
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

void UCardEffectHostComponent::HandleOwnerDamaged( ABuilding* building, int32 damage )
{
	DispatchTrigger( ECardTriggerReason::Damaged, nullptr );
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

	DispatchTrigger( ECardTriggerReason::HitLanded, target );

	if ( const IEntity* entity = Cast<IEntity>( target ) )
	{
		if ( !entity->Stats().IsAlive() )
		{
			DispatchTrigger( ECardTriggerReason::KillLanded, target );
		}
	}
}

void UCardEffectHostComponent::DispatchTrigger( ECardTriggerReason reason, AActor* instigator )
{
	if ( Active_.Num() == 0 )
	{
		return;
	}

	UCardSubsystem* subsystem = UCardSubsystem::Get( this );
	ABuilding* building = Cast<ABuilding>( GetOwner() );

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
		ctx.TriggerReason = reason;

		if ( rec.SourceCard->Events.IsValidIndex( rec.EventIndex ) )
		{
			const FCardEvent& event = rec.SourceCard->Events[rec.EventIndex];
			bool bConditionsPass = true;
			for ( const TObjectPtr<UCardCondition>& cond : event.Conditions )
			{
				if ( cond && !cond->IsMet( ctx ) )
				{
					bConditionsPass = false;
					break;
				}
			}
			if ( !bConditionsPass )
			{
				continue;
			}
		}

		rec.Effect->Execute( ctx );
	}
}
