// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "FollowTargetTask.generated.h"

/**
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UFollowTargetTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UFollowTargetTask();

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings" )
	bool bUsePathfinding = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings" )
	float ObservedTargetMovementTolerance = 10.0f;

	// private:
	// 	bool bPrevTargetSaved_ = false;
	// 	FVector PrevTargetLocation_;
};
