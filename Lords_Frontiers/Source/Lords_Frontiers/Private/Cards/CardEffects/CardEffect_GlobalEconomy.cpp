#include "Cards/CardEffects/CardEffect_GlobalEconomy.h"

#include "Cards/CardSubsystem.h"

void UCardEffect_GlobalEconomy::Apply_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, Delta );
}

void UCardEffect_GlobalEconomy::Revert_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, -Delta );
}

void UCardEffect_GlobalEconomy::ApplyDelta( const FCardEffectContext& context, int32 signedDelta ) const
{
	if ( signedDelta == 0 )
	{
		return;
	}

	UCardSubsystem* subsystem = context.Subsystem.Get();
	if ( !subsystem )
	{
		return;
	}

	switch ( Operation )
	{
	case ECardEconomyOp::ProductionBonus:
		subsystem->AddEconomyProductionBonus( ResourceTarget, signedDelta );
		break;
	case ECardEconomyOp::MaintenanceReduction:
		subsystem->AddEconomyMaintenanceReduction( ResourceTarget, signedDelta );
		break;
	default:
		break;
	}
}

FText UCardEffect_GlobalEconomy::GetDisplayText_Implementation() const
{
	const TCHAR* sign = Delta >= 0 ? TEXT( "+" ) : TEXT( "" );
	const TCHAR* opName =
		Operation == ECardEconomyOp::ProductionBonus ? TEXT( "production" ) : TEXT( "maintenance reduction" );

	return FText::FromString(
		FString::Printf( TEXT( "%s%d global %s %s" ),
			sign, Delta, *CardTypeHelpers::GetResourceName( ResourceTarget ), opName ) );
}
