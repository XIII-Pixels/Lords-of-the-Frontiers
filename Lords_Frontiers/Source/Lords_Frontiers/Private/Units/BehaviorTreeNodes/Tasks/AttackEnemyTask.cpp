// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Tasks/AttackEnemyTask.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Units/Unit.h"

UAttackEnemyTask::UAttackEnemyTask()
{
	NodeName = "Attack Enemy";
}

EBTNodeResult::Type UAttackEnemyTask::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	Super::ExecuteTask( ownerComp, nodeMemory );

	AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UAttackEnemyTask failed to get AIController" ) );
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboard = controller->GetBlackboardComponent();
	if ( !blackboard )
	{
		UE_LOG( LogTemp, Warning, TEXT("Task UAttackEnemyTask failed to get BlackboardComponent" ) );
		return EBTNodeResult::Failed;
	}

	auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UAttackEnemyTask failed to get AUnit" ) );
		return EBTNodeResult::Failed;
	}

	TObjectPtr<AActor> enemy = Cast<AActor>( blackboard->GetValueAsObject( GetSelectedBlackboardKey() ) );
	unit->Attack( enemy );
	
	return EBTNodeResult::Succeeded;
}
