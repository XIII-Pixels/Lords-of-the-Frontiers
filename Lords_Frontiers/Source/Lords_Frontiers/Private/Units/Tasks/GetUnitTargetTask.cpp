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

	auto unit = Cast<AUnit>( controller->GetPawn() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Task GetUnitTarget failed to get AUnit" ) );
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboard = controller->GetBlackboardComponent();
	if ( !blackboard )
	{
		UE_LOG( LogTemp, Warning, TEXT("Task GetUnitTarget failed to get BlackboardComponent" ) );
		return EBTNodeResult::Failed;
	}

	if ( TObjectPtr<AActor> targetActor = unit->Target() )
	{
		FVector targetLocation = targetActor->GetActorLocation();
		if ( blackboard->GetValueAsVector( GetSelectedBlackboardKey() ) == targetLocation )
		{
			return EBTNodeResult::InProgress;
		}

		if ( UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>( GetWorld() ) )
		{
			FNavLocation result;
			float& d = AcceptableDistanceAroundTarget;
			if ( navSys->ProjectPointToNavigation( targetLocation, result, FVector( d, d, d ) ) )
			{
				blackboard->SetValueAsVector( GetSelectedBlackboardKey(), result.Location );
				return EBTNodeResult::InProgress;
			}
		}
	}

	return EBTNodeResult::Failed;
}
