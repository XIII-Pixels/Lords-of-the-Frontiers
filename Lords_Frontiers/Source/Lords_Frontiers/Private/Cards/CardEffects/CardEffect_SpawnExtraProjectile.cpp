#include "Cards/CardEffects/CardEffect_SpawnExtraProjectile.h"

#include "Building/Building.h"
#include "Components/Attack/AttackRangedComponent.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardSpawnExtraProjectile, Log, All );

void UCardEffect_SpawnExtraProjectile::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::AttackFired &&
	     context.TriggerReason != ECardTriggerReason::HitLanded &&
	     context.TriggerReason != ECardTriggerReason::KillLanded )
	{
		return;
	}

	ABuilding* building = context.Building.Get();
	AActor* target = context.EventInstigator.Get();
	if ( !building )
	{
		UE_LOG( LogCardSpawnExtraProjectile, Warning, TEXT( "[%s] building is null, cannot fire" ), *GetName() );
		return;
	}
	if ( !target )
	{
		UE_LOG( LogCardSpawnExtraProjectile, Warning, TEXT( "[%s] target is null, cannot fire" ), *GetName() );
		return;
	}

	UAttackRangedComponent* attack = building->FindComponentByClass<UAttackRangedComponent>();
	if ( !attack )
	{
		UE_LOG( LogCardSpawnExtraProjectile, Warning,
			TEXT( "[%s] %s has no UAttackRangedComponent" ), *GetName(), *building->GetName() );
		return;
	}

	UE_LOG( LogCardSpawnExtraProjectile, Log,
		TEXT( "[%s] firing %d extra projectile(s) x%.2f on %s against %s" ),
		*GetName(), ExtraProjectileCount, DamageMultiplier,
		*building->GetName(), *target->GetName() );

	for ( int32 i = 0; i < ExtraProjectileCount; ++i )
	{
		attack->FireExtraProjectile( target, DamageMultiplier );
	}
}

FText UCardEffect_SpawnExtraProjectile::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf( TEXT( "+%d projectile(s) x%g damage" ), ExtraProjectileCount, DamageMultiplier ) );
}
