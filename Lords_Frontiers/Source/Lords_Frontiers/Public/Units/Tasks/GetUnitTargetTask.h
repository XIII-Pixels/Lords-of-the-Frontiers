// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "GetUnitTargetTask.generated.h"

/** (Gregory-hub)
 * Task for getting unit target point and saving it to blackboard */
UCLASS()
class LORDS_FRONTIERS_API UGetUnitTargetTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UGetUnitTargetTask();

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;

protected:
	UPROPERTY(EditAnywhere)
	float AcceptableDistanceAroundTarget = 100.0f;
};
