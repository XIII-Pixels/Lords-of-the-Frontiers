#include "Cards/CardEffects/CardEffect_DamageReflect.h"

#include "Building/Building.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffectHostComponent.h"
#include "Entity.h"
#include "EntityStats.h"

namespace
{
	FName MakeHitsKey( const UCardDataAsset* card )
	{
		const FString cardName = card ? card->GetName() : TEXT( "null" );
		return FName( *FString::Printf( TEXT( "%s#reflect_hits" ), *cardName ) );
	}
}

void UCardEffect_DamageReflect::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::Damaged )
	{
		return;
	}

	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	AActor* attacker = context.EventInstigator.Get();
	if ( !host || !building || !attacker )
	{
		return;
	}

	const FName hitsKey = MakeHitsKey( context.SourceCard.Get() );
	const int32 hits = host->IncrementCounter( hitsKey );
	const int32 every = FMath::Max( 1, EveryNthHit );
	if ( hits % every != 0 )
	{
		return;
	}

	IEntity* attackerEntity = Cast<IEntity>( attacker );
	if ( !attackerEntity || !attackerEntity->Stats().IsAlive() )
	{
		return;
	}

	const int32 damageTaken = FMath::Max( 1, building->Stats().MaxHealth() - building->Stats().Health() );
	const int32 reflect = FMath::Max( MinReflectDamage,
		FMath::RoundToInt( static_cast<float>( damageTaken ) * ReflectPercent / 100.f ) );

	attackerEntity->TakeDamage( reflect, building );
}

FText UCardEffect_DamageReflect::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf( TEXT( "Reflect %.0f%% every %d hit(s)" ), ReflectPercent, EveryNthHit ) );
}
