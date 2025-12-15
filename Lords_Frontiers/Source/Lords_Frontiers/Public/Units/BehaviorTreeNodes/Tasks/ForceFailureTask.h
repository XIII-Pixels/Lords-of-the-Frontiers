// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ForceFailureTask.generated.h"

/** (Gregory-hub)
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UForceFailureTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UForceFailureTask();
	
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
};
