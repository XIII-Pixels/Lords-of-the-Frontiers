// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Tasks/GetUnitTargetTask.h"

#include "Units/Unit.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UGetUnitTargetTask::UGetUnitTargetTask()
{
	NodeName = "Get Unit Target";
}

EBTNodeResult::Type UGetUnitTargetTask::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
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
		UE_LOG( LogTemp, Warning, TEXT("Task GetUnitTarget failed to get BlackboardComponent" ) );
		return EBTNodeResult::Failed;
	}
	
	auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task GetUnitTarget failed to get AUnit" ) );
		return EBTNodeResult::Failed;
	}

	if ( TObjectPtr<AActor> targetActor = unit->Target() )
	{
		blackboard->SetValueAsObject(GetSelectedBlackboardKey(), targetActor);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
