// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Tasks/FollowTargetTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UFollowTargetTask::UFollowTargetTask()
{
	NodeName = "Follow Target";
}

EBTNodeResult::Type UFollowTargetTask::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	Super::ExecuteTask( ownerComp, nodeMemory );

	AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MoveToTarget failed: no AIController" ) );
		return EBTNodeResult::Failed;
	}

	APawn* pawn = controller->GetPawn();
	if ( !pawn )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MoveToTarget failed: no Pawn" ) );
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboard = ownerComp.GetBlackboardComponent();
	if ( !blackboard )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MoveToTarget failed: no Blackboard" ) );
		return EBTNodeResult::Failed;
	}

	FVector target = blackboard->GetValueAsVector( GetSelectedBlackboardKey() );

	FAIMoveRequest request;
	request.SetGoalLocation( target );
	request.SetAcceptanceRadius( 0.0f );
	request.SetUsePathfinding( bUsePathfinding );

	FNavPathSharedPtr outPath;
	const FPathFollowingRequestResult result = controller->MoveTo( request, &outPath );

	if ( result == EPathFollowingRequestResult::RequestSuccessful )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MoveToTarget: MoveTo() returned RequestSuccessful" ) );
		return EBTNodeResult::Succeeded;
	}
	if ( result == EPathFollowingRequestResult::AlreadyAtGoal )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MoveToTarget: MoveTo() returned AlreadyAtGoal" ) );
		return EBTNodeResult::Succeeded;
	}
	if ( result == EPathFollowingRequestResult::Failed )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MoveToTarget: MoveTo() returned Failed" ) );
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Failed;
}
