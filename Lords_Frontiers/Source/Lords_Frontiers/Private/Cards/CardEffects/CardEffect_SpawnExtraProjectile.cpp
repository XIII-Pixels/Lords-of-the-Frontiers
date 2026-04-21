#include "Cards/CardEffects/CardEffect_SpawnExtraProjectile.h"

#include "Building/Building.h"
#include "Components/Attack/AttackRangedComponent.h"

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
	if ( !building || !target )
	{
		return;
	}

	UAttackRangedComponent* attack = building->FindComponentByClass<UAttackRangedComponent>();
	if ( !attack )
	{
		return;
	}

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
