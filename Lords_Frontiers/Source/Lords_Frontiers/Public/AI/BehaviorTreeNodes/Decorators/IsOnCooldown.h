// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"

#include "CoreMinimal.h"

#include "IsOnCooldown.generated.h"

/** (Gregory-hub)
 * Check if entity is on cooldown */
UCLASS()
class LORDS_FRONTIERS_API UIsOnCooldown : public UBTDecorator
{
	GENERATED_BODY()

public:
	UIsOnCooldown();

	virtual bool CalculateRawConditionValue( UBehaviorTreeComponent& ownerComp, uint8* nodeMemory ) const override;
};
