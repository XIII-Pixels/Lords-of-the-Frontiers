// Fill out your copyright notice in the Description page of Project Settings.

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/BehaviorTreeNodes/Tasks/Debug/FindRandomLocationTask.h"

#include "NavigationSystem.h"

UFindRandomLocationTask::UFindRandomLocationTask()
{
	NodeName = "Find Random Location";
}

/** (Gregory-hub)
 * Find random location and save to blackboard */
EBTNodeResult::Type UFindRandomLocationTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		return EBTNodeResult::Failed;
	}

	APawn* pawn = controller->GetPawn();
	if ( !pawn )
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboard = ownerComp.GetBlackboardComponent();
	if ( !blackboard )
	{
		return EBTNodeResult::Failed;
	}

	// Generate random point in circle around pawn and save to blackboard
	FVector pawnLocation = pawn->GetActorLocation();
	if ( auto* const navSystem = UNavigationSystemV1::GetCurrent( GetWorld() ) )
	{
		FNavLocation location;
		if ( navSystem->GetRandomPointInNavigableRadius( pawnLocation, SearchRadius_, location ) )
		{
			blackboard->SetValueAsVector( GetSelectedBlackboardKey(), location.Location );

			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
