// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"

#include "CoreMinimal.h"

#include "DoesSeeEnemyDecorator.generated.h"

/** (Gregory-hub)
 * Check if unit sees something it can attack */
UCLASS()
class LORDS_FRONTIERS_API UDoesSeeEnemyDecorator : public UBTDecorator
{
	GENERATED_BODY()

public:
	UDoesSeeEnemyDecorator();

	virtual bool CalculateRawConditionValue( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory ) const override;
};
