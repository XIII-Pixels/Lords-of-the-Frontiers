#include "Cards/CardEffects/CardEffect_RuinAura.h"

#include "Building/Building.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Cards/Visuals/CardAoEDebug.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Utilities/TraceChannelMappings.h"

#include "Engine/OverlapResult.h"
#include "Engine/World.h"

void UCardEffect_RuinAura::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::AuraTick )
	{
		return;
	}

	ABuilding* building = context.Building.Get();
	if ( !building || !building->IsRuined() )
	{
		return;
	}

	UWorld* world = building->GetWorld();
	if ( !world || Radius <= 0.f )
	{
		return;
	}

	const IEntity* ownerEntity = Cast<IEntity>( building );
	const ETeam ownerTeam = ownerEntity ? ownerEntity->Team() : ETeam::Cat;

	const FVector center = building->GetActorLocation();

	if ( bDebugDrawRadius )
	{
		CardAoEDebug::DrawRadius( building, center, Radius, 0.6f, DebugColor );
	}

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params;
	params.AddIgnoredActor( building );

	world->OverlapMultiByChannel(
		overlaps, center, FQuat::Identity,
		ECC_Entity, FCollisionShape::MakeSphere( Radius ), params );

	TSet<AActor*> seen;
	for ( const FOverlapResult& result : overlaps )
	{
		AActor* hitActor = result.GetActor();
		if ( !hitActor || seen.Contains( hitActor ) )
		{
			continue;
		}
		seen.Add( hitActor );

		IEntity* enemy = Cast<IEntity>( hitActor );
		if ( !enemy || !enemy->Stats().IsAlive() || enemy->Team() == ownerTeam )
		{
			continue;
		}

		if ( DamagePerTick > 0 )
		{
			enemy->TakeDamage( DamagePerTick, building );
		}

		if ( StatusToApply )
		{
			if ( UStatusEffectTracker* tracker = UStatusEffectTracker::EnsureOn( hitActor ) )
			{
				tracker->ApplyStatus( StatusToApply, building );
			}
		}
	}
}

FText UCardEffect_RuinAura::GetDisplayText_Implementation() const
{
	FString damagePart = DamagePerTick > 0
		? FString::Printf( TEXT( "%d dmg/tick" ), DamagePerTick )
		: FString();

	FString statusPart = StatusToApply
		? FString::Printf( TEXT( " + %s" ), *StatusToApply->StatusTag.ToString() )
		: FString();

	return FText::FromString(
		FString::Printf( TEXT( "Ruin aura %.0f: %s%s" ), Radius, *damagePart, *statusPart ) );
}
