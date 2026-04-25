// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttackRangedComponent.h"

#include "CoreMinimal.h"

#include "UnitAttackRangedComponent.generated.h"

UCLASS( ClassGroup = ( Attack ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UUnitAttackRangedComponent : public UAttackRangedComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void SetAttackMode() override;
};
