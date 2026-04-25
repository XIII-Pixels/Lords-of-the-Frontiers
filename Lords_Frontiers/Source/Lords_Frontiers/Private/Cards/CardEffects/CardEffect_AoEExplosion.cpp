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

DEFINE_LOG_CATEGORY_STATIC( LogCardAoEExplosion, Log, All );

void UCardEffect_AoEExplosion::Execute_Implementation( const FCardEffectContext& context )
{
	ABuilding* ownerBuilding = context.Building.Get();
	AActor* instigatorActor = context.EventInstigator.Get();

	if ( !ownerBuilding )
	{
		UE_LOG( LogCardAoEExplosion, Warning, TEXT( "[%s] ownerBuilding is null" ), *GetName() );
		return;
	}

	FVector center = FVector::ZeroVector;
	if ( CenterOrigin == EAoECenterOrigin::OwnerBuilding )
	{
		center = ownerBuilding->GetActorLocation();
	}
	else if ( instigatorActor )
	{
		center = instigatorActor->GetActorLocation();
	}
	else if ( context.bHasEventLocation )
	{
		center = context.EventLocation;
	}
	else
	{
		UE_LOG( LogCardAoEExplosion, Warning,
			TEXT( "[%s] no center available — explosion skipped" ), *GetName() );
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
	if ( instigatorActor )
	{
		params.AddIgnoredActor( instigatorActor );
	}

	if ( bDebugDrawRadius )
	{
		CardAoEDebug::DrawRadius( ownerBuilding, center, Radius, DebugDrawDuration, DebugColor );
	}

	const bool bAny = world->OverlapMultiByChannel(
		overlaps, center, FQuat::Identity,
		ECC_Entity, FCollisionShape::MakeSphere( Radius ), params );

	UE_LOG( LogCardAoEExplosion, Log,
		TEXT( "[%s] center=%s radius=%.1f finalDamage=%d overlaps=%d" ),
		*GetName(), *center.ToCompactString(),
		Radius, finalDamage, overlaps.Num() );

	if ( !bAny )
	{
		return;
	}

	TSet<AActor*> alreadyAffected;
	int32 affected = 0;
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
			enemy->TakeDamage( finalDamage, ownerBuilding );
		}

		if ( StatusToApply )
		{
			if ( UStatusEffectTracker* tracker = UStatusEffectTracker::EnsureOn( hitActor ) )
			{
				tracker->ApplyStatus( StatusToApply, ownerBuilding );
			}
		}
		++affected;
	}

	UE_LOG( LogCardAoEExplosion, Log,
		TEXT( "[%s] affected %d enemies" ), *GetName(), affected );
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
