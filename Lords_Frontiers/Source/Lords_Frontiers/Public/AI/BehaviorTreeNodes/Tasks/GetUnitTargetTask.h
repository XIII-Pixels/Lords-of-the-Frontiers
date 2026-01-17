// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "CoreMinimal.h"

#include "GetUnitTargetTask.generated.h"

/** (Gregory-hub)
 * Task for getting target point to follow and saving it to blackboard */
UCLASS()
class LORDS_FRONTIERS_API UGetUnitTargetTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UGetUnitTargetTask();

	virtual EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory ) override;
};
