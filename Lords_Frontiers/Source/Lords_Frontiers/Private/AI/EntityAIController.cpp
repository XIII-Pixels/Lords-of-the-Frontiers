// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/EntityAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "ControlledByTree.h"

void AEntityAIController::OnPossess( APawn* pawn )
{
	Super::OnPossess( pawn );

	// Start behavior tree
	if ( const auto actor = Cast<IControlledByTree>( pawn ) )
	{
		if ( const auto behaviorTree = actor->BehaviorTree() )
		{
			UBlackboardComponent* blackboard;
			UseBlackboard( behaviorTree->GetBlackboardAsset(), blackboard );
			Blackboard = blackboard;
			RunBehaviorTree( behaviorTree );
		}
	}
}
