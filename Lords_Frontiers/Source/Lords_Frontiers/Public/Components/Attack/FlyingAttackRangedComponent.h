// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Attack/AttackRangedComponent.h"
#include "CoreMinimal.h"

#include "FlyingAttackRangedComponent.generated.h"

UCLASS( ClassGroup = ( Attack ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UFlyingAttackRangedComponent : public UAttackRangedComponent
{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Flying" )
	bool bOnlyAttackGoalBuilding_ = true;

protected:
	virtual bool IsAttackable( TObjectPtr<AActor> enemyActor ) const override;
};