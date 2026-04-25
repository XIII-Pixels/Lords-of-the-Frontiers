#include "Cards/CardEffects/CardEffect_SpawnShrapnel.h"

#include "Building/Building.h"
#include "Components/Attack/AttackRangedComponent.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardSpawnShrapnel, Log, All );

namespace
{
	bool GIsFiringShrapnel = false;
}

void UCardEffect_SpawnShrapnel::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::HitLanded &&
	     context.TriggerReason != ECardTriggerReason::KillLanded &&
	     context.TriggerReason != ECardTriggerReason::Landed )
	{
		return;
	}

	if ( GIsFiringShrapnel )
	{
		return;
	}

	ABuilding* building = context.Building.Get();
	if ( !building )
	{
		return;
	}

	UAttackRangedComponent* attack = building->FindComponentByClass<UAttackRangedComponent>();
	if ( !attack )
	{
		UE_LOG( LogCardSpawnShrapnel, Warning,
			TEXT( "[%s] %s has no UAttackRangedComponent — cannot fire shards" ),
			*GetName(), *building->GetName() );
		return;
	}

	FVector spawnLocation = FVector::ZeroVector;
	if ( context.bHasEventLocation )
	{
		spawnLocation = context.EventLocation;
	}
	else if ( AActor* victim = context.EventInstigator.Get() )
	{
		spawnLocation = victim->GetActorLocation();
	}
	else
	{
		spawnLocation = building->GetActorLocation();
	}

	UE_LOG( LogCardSpawnShrapnel, Log,
		TEXT( "[%s] %d shards from %s damageMul=%.2f spread=%.0f range=%.0f" ),
		*GetName(), ShrapnelCount, *spawnLocation.ToCompactString(),
		DamageMultiplier, SpreadDegrees, Range );

	TGuardValue<bool> reentryGuard( GIsFiringShrapnel, true );
	attack->FireShrapnel(
		spawnLocation, ShrapnelCount, DamageMultiplier,
		SpreadDegrees, Range, Speed, ShardSplashRadius, ShardProjectileClass );
}

FText UCardEffect_SpawnShrapnel::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf(
			TEXT( "%d shards from impact (%.0f%% dmg, %.0f° spread)" ),
			ShrapnelCount, DamageMultiplier * 100.f, SpreadDegrees ) );
}
