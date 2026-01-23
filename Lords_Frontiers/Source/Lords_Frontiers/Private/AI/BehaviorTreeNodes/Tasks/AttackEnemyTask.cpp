// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Tasks/AttackEnemyTask.h"

#include "AIController.h"
#include "Attacker.h"
#include "BehaviorTree/BlackboardComponent.h"

UAttackEnemyTask::UAttackEnemyTask()
{
	NodeName = "Attack Enemy";
}

EBTNodeResult::Type UAttackEnemyTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	Super::ExecuteTask( ownerComp, nodeMemory );

	AAIController* controller = ownerComp.GetAIOwner();
	if ( !controller )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UAttackEnemyTask failed to get AIController" ) );
		return EBTNodeResult::Failed;
	}

	const UBlackboardComponent* blackboard = controller->GetBlackboardComponent();
	if ( !blackboard )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UAttackEnemyTask failed to get BlackboardComponent" ) );
		return EBTNodeResult::Failed;
	}

	const auto attacker = Cast<IAttacker>( controller->GetPawn() );
	if ( !attacker )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UAttackEnemyTask failed to get IAttacker" ) );
		return EBTNodeResult::Failed;
	}

	const TObjectPtr<AActor> enemy = Cast<AActor>( blackboard->GetValueAsObject( GetSelectedBlackboardKey() ) );
	attacker->Attack( enemy );

	return EBTNodeResult::Succeeded;
}
