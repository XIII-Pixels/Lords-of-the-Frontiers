// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Tasks/GetEnemyInSightTask.h"

#include "AIController.h"
#include "Attacker.h"
#include "BehaviorTree/BlackboardComponent.h"

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

	const auto attacker = Cast<IAttacker>( controller->GetPawn() );
	if ( !attacker )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task UAttackEnemyTask failed to get IAttacker" ) );
		return EBTNodeResult::Failed;
	}

	blackboard->SetValueAsObject( GetSelectedBlackboardKey(), attacker->EnemyInSight() );

	return EBTNodeResult::Succeeded;
}
