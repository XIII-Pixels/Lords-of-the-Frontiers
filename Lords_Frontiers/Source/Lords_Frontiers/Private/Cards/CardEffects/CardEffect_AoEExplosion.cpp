#include "Cards/CardEffects/CardEffect_AoEExplosion.h"

#include "Building/Building.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Cards/Visuals/CardAoEDebug.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Utilities/TraceChannelMappings.h"

#include "Engine/OverlapResult.h"
#include "Engine/World.h"

void UCardEffect_AoEExplosion::Execute_Implementation( const FCardEffectContext& context )
{
	ABuilding* ownerBuilding = context.Building.Get();
	AActor* instigatorActor = context.EventInstigator.Get();

	AActor* centerActor = CenterOrigin == EAoECenterOrigin::OwnerBuilding ? Cast<AActor>( ownerBuilding ) : instigatorActor;
	if ( !centerActor || !ownerBuilding )
	{
		return;
	}

	UWorld* world = ownerBuilding->GetWorld();
	if ( !world )
	{
		return;
	}

	const IEntity* ownerEntity = Cast<IEntity>( ownerBuilding );
	const ETeam ownerTeam = ownerEntity ? ownerEntity->Team() : ETeam::Cat;

	int32 finalDamage = Damage;
	if ( DamageMultiplierOfOwner > 0.f && ownerEntity )
	{
		finalDamage += FMath::RoundToInt(
			static_cast<float>( ownerEntity->Stats().AttackDamage() ) * DamageMultiplierOfOwner );
	}

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params;
	params.AddIgnoredActor( ownerBuilding );
	params.AddIgnoredActor( centerActor );

	const FVector center = centerActor->GetActorLocation();

	if ( bDebugDrawRadius )
	{
		CardAoEDebug::DrawRadius( ownerBuilding, center, Radius, DebugDrawDuration, DebugColor );
	}

	const bool bAny = world->OverlapMultiByChannel(
		overlaps, center, FQuat::Identity,
		ECC_Entity, FCollisionShape::MakeSphere( Radius ), params );

	if ( !bAny )
	{
		return;
	}

	TSet<AActor*> alreadyAffected;
	for ( const FOverlapResult& result : overlaps )
	{
		AActor* hitActor = result.GetActor();
		if ( !hitActor || alreadyAffected.Contains( hitActor ) )
		{
			continue;
		}
		alreadyAffected.Add( hitActor );

		IEntity* enemy = Cast<IEntity>( hitActor );
		if ( !enemy || !enemy->Stats().IsAlive() || enemy->Team() == ownerTeam )
		{
			continue;
		}

		if ( finalDamage > 0 )
		{
			enemy->TakeDamage( finalDamage );
		}

		if ( StatusToApply )
		{
			if ( UStatusEffectTracker* tracker = UStatusEffectTracker::EnsureOn( hitActor ) )
			{
				tracker->ApplyStatus( StatusToApply );
			}
		}
	}
}

FText UCardEffect_AoEExplosion::GetDisplayText_Implementation() const
{
	FString damagePart;
	if ( Damage > 0 || DamageMultiplierOfOwner > 0.f )
	{
		if ( DamageMultiplierOfOwner > 0.f )
		{
			damagePart = FString::Printf( TEXT( "+%g dmg" ), DamageMultiplierOfOwner );
		}
		else
		{
			damagePart = FString::Printf( TEXT( "%d dmg" ), Damage );
		}
	}

	const FString statusPart = StatusToApply
		? FString::Printf( TEXT( " + %s" ), *StatusToApply->StatusTag.ToString() )
		: FString();

	return FText::FromString(
		FString::Printf( TEXT( "AoE %.0f%s%s" ), Radius, *damagePart, *statusPart ) );
}
