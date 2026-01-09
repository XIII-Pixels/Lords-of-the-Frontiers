// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Tasks/GetEnemyInSightTask.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Units/Unit.h"

UGetEnemyInSightTask::UGetEnemyInSightTask()
{
	NodeName = "Get Enemy in Sight";
}

EBTNodeResult::Type UGetEnemyInSightTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	Super::ExecuteTask( ownerComp, nodeMemory );

	AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UGetEnemyInSightTask failed to get AIController" ) );
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboard = controller->GetBlackboardComponent();
	if ( !blackboard )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UGetEnemyInSightTask failed to get BlackboardComponent" ) );
		return EBTNodeResult::Failed;
	}

	auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UGetEnemyInSightTask failed to get AUnit" ) );
		return EBTNodeResult::Failed;
	}

	blackboard->SetValueAsObject( GetSelectedBlackboardKey(), unit->EnemyInSight() );

	return EBTNodeResult::Succeeded;
}
