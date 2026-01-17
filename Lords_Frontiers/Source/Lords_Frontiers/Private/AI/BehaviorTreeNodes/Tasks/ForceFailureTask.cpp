// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Tasks/ForceFailureTask.h"

UForceFailureTask::UForceFailureTask()
{
	NodeName = "Force Failure";
}

EBTNodeResult::Type UForceFailureTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	Super::ExecuteTask( ownerComp, nodeMemory );
	return EBTNodeResult::Failed;
}
