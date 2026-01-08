// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Decorators/DoesSeeEnemyDecorator.h"

#include "Units/Unit.h"

UDoesSeeEnemyDecorator::UDoesSeeEnemyDecorator()
{
	NodeName = "Does See Enemy";
}

bool UDoesSeeEnemyDecorator::CalculateRawConditionValue( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory ) const
{
	const AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UDoesSeeEnemyDecorator failed to get AIController" ) );
		return false;
	}

	const auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UDoesSeeEnemyDecorator failed to get AUnit" ) );
		return false;
	}

	if ( unit->EnemyInSight() != nullptr )
	{
		return true;
	}
	return false;
}
