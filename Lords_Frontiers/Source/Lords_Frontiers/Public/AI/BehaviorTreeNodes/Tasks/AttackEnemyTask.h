// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "CoreMinimal.h"

#include "AttackEnemyTask.generated.h"

/** (Gregory-hub)
 * Task for dealing damage to enemy */
UCLASS()
class LORDS_FRONTIERS_API UAttackEnemyTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UAttackEnemyTask();

	virtual EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory ) override;
};
