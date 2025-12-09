// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "FollowTargetTask.generated.h"

/** (Gregory-hub)
 * Task for following target */
UCLASS()
class LORDS_FRONTIERS_API UFollowTargetTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UFollowTargetTask();

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	bool bUsePathfinding = true;
};
