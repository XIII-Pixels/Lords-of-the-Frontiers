// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Tasks/StartFollowingTask.h"

#include "Units/Unit.h"

UStartFollowingTask::UStartFollowingTask()
{
	NodeName = "Start Following Target";
}

EBTNodeResult::Type UStartFollowingTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
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

	unit->StartFollowing();
	return EBTNodeResult::Succeeded;
}
