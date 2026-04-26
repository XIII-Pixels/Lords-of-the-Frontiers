#include "Cards/CardEffects/CardEffect_PrimeNextShotDamageBonus.h"

#include "Building/Building.h"
#include "Components/Attack/AttackRangedComponent.h"

void UCardEffect_PrimeNextShotDamageBonus::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::BeforeAttackFire )
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
		return;
	}

	attack->AddPendingDamageBonus( DamageBonusPercent );
}

FText UCardEffect_PrimeNextShotDamageBonus::GetDisplayText_Implementation() const
{
	return FText::FromString(
		FString::Printf( TEXT( "+%g%% damage on next shot" ), DamageBonusPercent ) );
}
