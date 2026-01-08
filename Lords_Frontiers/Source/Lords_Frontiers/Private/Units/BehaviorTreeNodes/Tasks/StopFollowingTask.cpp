// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Tasks/StopFollowingTask.h"

#include "Units/Unit.h"

UStopFollowingTask::UStopFollowingTask()
{
	NodeName = "Stop Following Target";
}

EBTNodeResult::Type UStopFollowingTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	Super::ExecuteTask( ownerComp, nodeMemory );

	const AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UStartFollowingTask failed to get AIController" ) );
		return EBTNodeResult::Failed;
	}

	const auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UStartFollowingTask failed to get AUnit" ) );
		return EBTNodeResult::Failed;
	}

	unit->StopFollowing();
	return EBTNodeResult::Succeeded;
}
