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
	     context.TriggerReason != ECardTriggerReason::AttackFired )
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
	AActor* centerActor = CenterOrigin == EAoECenterOrigin::OwnerBuilding ? Cast<AActor>( building ) : victim;
	if ( !centerActor )
	{
		return;
	}

	UWorld* world = centerActor->GetWorld();
	if ( !world )
	{
		return;
	}

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = building;

	ACardAoEField* field = world->SpawnActor<ACardAoEField>(
		FieldClass, centerActor->GetActorLocation(), FRotator::ZeroRotator, params );
	if ( !field )
	{
		return;
	}

	field->Initialize( building, Radius, Duration, TickInterval, DamagePerTick, StatusPerTick );
}

FText UCardEffect_SpawnAoEField::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf( TEXT( "AoE field r=%.0f for %.1fs" ), Radius, Duration ) );
}
