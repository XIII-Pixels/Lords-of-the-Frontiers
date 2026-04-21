#include "Cards/CardEffects/CardEffect_AuraStacker.h"

#include "Building/Building.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/CardEffects/CardEffect_StatModifier.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Utilities/TraceChannelMappings.h"

#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"

namespace
{
	const FName AppliedMilliTag( TEXT( "aura_applied_milli" ) );

	FNumericProperty* FindNumericProperty( FName statName )
	{
		if ( statName.IsNone() )
		{
			return nullptr;
		}
		FProperty* prop = FEntityStats::StaticStruct()->FindPropertyByName( statName );
		return CastField<FNumericProperty>( prop );
	}

	void ApplyStatDelta( ABuilding* building, FName statName, float delta )
	{
		if ( !building || FMath::IsNearlyZero( delta ) )
		{
			return;
		}
		FNumericProperty* prop = FindNumericProperty( statName );
		if ( !prop )
		{
			return;
		}
		FEntityStats& stats = building->Stats();
		void* addr = prop->ContainerPtrToValuePtr<void>( &stats );
		const double current = prop->GetFloatingPointPropertyValue( addr );
		prop->SetFloatingPointPropertyValue( addr, current + delta );
	}
}

void UCardEffect_AuraStacker::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::AuraTick )
	{
		return;
	}

	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building || StatName.IsNone() )
	{
		return;
	}

	UWorld* world = building->GetWorld();
	if ( !world )
	{
		return;
	}

	float searchRadius = Radius;
	if ( searchRadius <= 0.f )
	{
		if ( const IEntity* ownerEntity = Cast<IEntity>( building ) )
		{
			searchRadius = ownerEntity->Stats().AttackRange();
		}
	}
	if ( searchRadius <= 0.f )
	{
		return;
	}

	const IEntity* ownerEntity = Cast<IEntity>( building );
	const ETeam ownerTeam = ownerEntity ? ownerEntity->Team() : ETeam::Cat;

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params;
	params.AddIgnoredActor( building );

	world->OverlapMultiByChannel(
		overlaps, building->GetActorLocation(), FQuat::Identity,
		ECC_Entity, FCollisionShape::MakeSphere( searchRadius ), params );

	int32 matchCount = 0;
	TSet<AActor*> counted;
	for ( const FOverlapResult& result : overlaps )
	{
		AActor* hitActor = result.GetActor();
		if ( !hitActor || counted.Contains( hitActor ) )
		{
			continue;
		}
		counted.Add( hitActor );

		const IEntity* enemy = Cast<IEntity>( hitActor );
		if ( !enemy || !enemy->Stats().IsAlive() || enemy->Team() == ownerTeam )
		{
			continue;
		}

		if ( RequiredStatus )
		{
			const UStatusEffectTracker* tracker = hitActor->FindComponentByClass<UStatusEffectTracker>();
			if ( !tracker || !tracker->HasStatus( RequiredStatus ) )
			{
				continue;
			}
		}

		matchCount++;
	}

	float desired = static_cast<float>( matchCount ) * StepPerEnemy;
	if ( MaxAccumulated > 0.f )
	{
		desired = FMath::Min( desired, MaxAccumulated );
	}

	const FName appliedKey = UCardEffectHostComponent::MakeCounterKey(
		context.SourceCard.Get(), context.EventIndex, AppliedMilliTag );

	const int32 appliedMilli = host->GetCounter( appliedKey );
	const float appliedNow = static_cast<float>( appliedMilli ) / 1000.f;
	const float diff = desired - appliedNow;

	if ( FMath::IsNearlyZero( diff, 1e-3f ) )
	{
		return;
	}

	ApplyStatDelta( building, StatName, diff );
	host->SetCounter( appliedKey, FMath::RoundToInt( desired * 1000.f ) );
}

void UCardEffect_AuraStacker::Revert_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building )
	{
		return;
	}

	const FName appliedKey = UCardEffectHostComponent::MakeCounterKey(
		context.SourceCard.Get(), context.EventIndex, AppliedMilliTag );

	const int32 appliedMilli = host->GetCounter( appliedKey );
	if ( appliedMilli != 0 )
	{
		ApplyStatDelta( building, StatName, -static_cast<float>( appliedMilli ) / 1000.f );
		host->SetCounter( appliedKey, 0 );
	}
}

FText UCardEffect_AuraStacker::GetDisplayText_Implementation() const
{
	if ( StatName.IsNone() )
	{
		return FText::GetEmpty();
	}

	FString statDisplay = StatName.ToString();
	if ( FProperty* prop = FEntityStats::StaticStruct()->FindPropertyByName( StatName ) )
	{
		statDisplay = prop->GetDisplayNameText().ToString();
	}

	const FString statusText = RequiredStatus
		? FString::Printf( TEXT( " w/ %s" ), *RequiredStatus->StatusTag.ToString() )
		: FString( TEXT( "" ) );

	return FText::FromString(
		FString::Printf( TEXT( "Aura: +%g %s per enemy in range%s (max %g)" ),
			StepPerEnemy, *statDisplay, *statusText, MaxAccumulated ) );
}

TArray<FString> UCardEffect_AuraStacker::GetModifiableStatNames()
{
	return UCardEffect_StatModifier::GetModifiableStatNames();
}
