// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "FindRandomLocationTask.generated.h"

/** (Gregory-hub)
 * Debug task. Is used to find random location for a pawn to move to */
UCLASS()
class LORDS_FRONTIERS_API UFindRandomLocationTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	explicit UFindRandomLocationTask();

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;

private:
	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	float SearchRadius_ = 15000.0f;
};
