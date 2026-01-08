// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/UnitAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "Units/Unit.h"

void AUnitAIController::Tick( float DeltaSeconds )
{
	Super::Tick( DeltaSeconds );
}

void AUnitAIController::OnPossess( APawn* pawn )
{
	Super::OnPossess( pawn );

	// (Gregory-hub)
	// Start behavior tree
	if ( auto unit = Cast<AUnit>( pawn ) )
	{
		if ( auto behaviorTree = unit->BehaviorTree() )
		{
			UBlackboardComponent* blackboard;
			UseBlackboard( behaviorTree->GetBlackboardAsset(), blackboard );
			Blackboard = blackboard;
			RunBehaviorTree( behaviorTree );
		}
	}
}
