// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BehaviorTreeNodes/Tasks/ForceSuccessTask.h"

UForceSuccessTask::UForceSuccessTask()
{
	NodeName = "Force Success";
}

EBTNodeResult::Type UForceSuccessTask::ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory )
{
	Super::ExecuteTask( ownerComp, nodeMemory );
	return EBTNodeResult::Succeeded;
}
