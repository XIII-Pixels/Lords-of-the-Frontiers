// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "DoesSeeAttackable.generated.h"

/** (Gregory-hub)
 * Check if unit sees something it can attack */
UCLASS()
class LORDS_FRONTIERS_API UDoesSeeAttackable : public UBTDecorator
{
	GENERATED_BODY()

public:
	UDoesSeeAttackable();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
