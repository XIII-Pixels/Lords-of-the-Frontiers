// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Tasks/GetUnitTargetTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Units/Unit.h"

#include "NavigationSystem.h"

UGetUnitTargetTask::UGetUnitTargetTask()
{
	NodeName = "Get Unit Target";
}

EBTNodeResult::Type UGetUnitTargetTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	Super::ExecuteTask( ownerComp, nodeMemory );

	AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task GetUnitTarget failed to get AIController" ) );
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboard = controller->GetBlackboardComponent();
	if ( !blackboard )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task GetUnitTarget failed to get BlackboardComponent" ) );
		return EBTNodeResult::Failed;
	}

	const auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task GetUnitTarget failed to get AUnit" ) );
		return EBTNodeResult::Failed;
	}

	const TWeakObjectPtr<AActor> targetActor = unit->FollowedTarget();
	if ( targetActor.IsValid() )
	{
		if ( targetActor->GetActorLocation() != blackboard->GetValueAsVector( GetSelectedBlackboardKey() ) )
		{
			blackboard->SetValueAsVector( GetSelectedBlackboardKey(), targetActor->GetActorLocation() );
			return EBTNodeResult::Succeeded;
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}
	return EBTNodeResult::Failed;
}
