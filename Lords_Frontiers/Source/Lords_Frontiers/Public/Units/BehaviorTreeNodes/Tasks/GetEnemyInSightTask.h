// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "GetEnemyInSightTask.generated.h"

/** (Gregory-hub)
 * Saves unit's enemy in sight to blackboard */
UCLASS()
class LORDS_FRONTIERS_API UGetEnemyInSightTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UGetEnemyInSightTask();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
};
