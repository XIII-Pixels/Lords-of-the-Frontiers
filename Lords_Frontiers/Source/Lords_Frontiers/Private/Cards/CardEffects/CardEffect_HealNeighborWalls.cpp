#include "Cards/CardEffects/CardEffect_HealNeighborWalls.h"

#include "Building/AdditiveBuilding.h"
#include "Building/Building.h"
#include "EntityStats.h"

#include "Engine/World.h"
#include "EngineUtils.h"

void UCardEffect_HealNeighborWalls::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::HitLanded &&
	     context.TriggerReason != ECardTriggerReason::AttackFired )
	{
		return;
	}

	ABuilding* owner = context.Building.Get();
	if ( !owner )
	{
		return;
	}

	UWorld* world = owner->GetWorld();
	if ( !world )
	{
		return;
	}

	const FVector origin = owner->GetActorLocation();

	AAdditiveBuilding* closest = nullptr;
	float closestDistSq = FLT_MAX;

	const float radiusSq = Radius * Radius;

	for ( TActorIterator<AAdditiveBuilding> it( world ); it; ++it )
	{
		AAdditiveBuilding* wall = *it;
		if ( !wall || wall->IsRuined() )
		{
			continue;
		}
		const float distSq = FVector::DistSquared( origin, wall->GetActorLocation() );
		if ( distSq > radiusSq )
		{
			continue;
		}

		if ( !bHealClosestOnly )
		{
			wall->Stats().Heal( HealAmount );
			continue;
		}

		if ( distSq < closestDistSq )
		{
			closestDistSq = distSq;
			closest = wall;
		}
	}

	if ( bHealClosestOnly && closest )
	{
		closest->Stats().Heal( HealAmount );
	}
}

FText UCardEffect_HealNeighborWalls::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf( TEXT( "Heal %s wall(s) in %.0f: +%d HP" ),
			bHealClosestOnly ? TEXT( "closest" ) : TEXT( "all" ),
			Radius, HealAmount ) );
}
