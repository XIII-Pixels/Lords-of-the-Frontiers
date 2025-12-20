// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ForceSuccessTask.generated.h"

/** (Gregory-hub) */
UCLASS()
class LORDS_FRONTIERS_API UForceSuccessTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UForceSuccessTask();
	
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
};
