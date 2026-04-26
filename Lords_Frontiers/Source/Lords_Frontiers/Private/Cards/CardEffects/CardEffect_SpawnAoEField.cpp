#include "Cards/CardEffects/CardEffect_SpawnAoEField.h"

#include "Building/Building.h"
#include "Cards/Feedback/CardAoEField.h"
#include "Cards/StatusEffects/StatusEffectDef.h"

#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardSpawnAoEField, Log, All );

void UCardEffect_SpawnAoEField::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::HitLanded &&
	     context.TriggerReason != ECardTriggerReason::KillLanded &&
	     context.TriggerReason != ECardTriggerReason::AttackFired &&
	     context.TriggerReason != ECardTriggerReason::Missed &&
	     context.TriggerReason != ECardTriggerReason::Landed )
	{
		return;
	}

	if ( !FieldClass )
	{
		UE_LOG( LogCardSpawnAoEField, Warning,
			TEXT( "SpawnAoEField: FieldClass is not set on effect — skipping" ) );
		return;
	}

	ABuilding* building = context.Building.Get();
	AActor* victim = context.EventInstigator.Get();

	FVector spawnLocation = FVector::ZeroVector;
	UWorld* world = nullptr;
	if ( CenterOrigin == EAoECenterOrigin::OwnerBuilding )
	{
		if ( !building )
		{
			return;
		}
		spawnLocation = building->GetActorLocation();
		world = building->GetWorld();
	}
	else if ( victim )
	{
		spawnLocation = victim->GetActorLocation();
		world = victim->GetWorld();
	}
	else if ( context.bHasEventLocation )
	{
		spawnLocation = context.EventLocation;
		world = building ? building->GetWorld() : nullptr;
	}
	else
	{
		return;
	}

	if ( !world )
	{
		return;
	}

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = building;

	ACardAoEField* field = world->SpawnActor<ACardAoEField>(
		FieldClass, spawnLocation, FRotator::ZeroRotator, params );
	if ( !field )
	{
		return;
	}

	field->Initialize(
		building, Radius, Duration, TickInterval, DamagePerTick, StatusPerTick,
		bDebugDrawRadius, DebugColor );
}

FText UCardEffect_SpawnAoEField::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf( TEXT( "AoE field r=%.0f for %.1fs" ), Radius, Duration ) );
}
