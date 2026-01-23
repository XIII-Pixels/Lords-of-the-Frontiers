// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Decorators/DoesSeeEnemyDecorator.h"

#include "AIController.h"
#include "Attacker.h"

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

	const auto attacker = Cast<IAttacker>( controller->GetPawn() );
	if ( !attacker )
	{
		UE_LOG( LogTemp, Warning, TEXT( "UDoesSeeEnemyDecorator failed to get IAttacker" ) );
		return false;
	}

	if ( attacker->EnemyInSight() )
	{
		return true;
	}

	return false;
}
