// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttackComponent.h"

#include "CoreMinimal.h"

#include "AttackMeleeComponent.generated.h"

class IEntity;

/** (Gregory-hub)
 * Makes actor attack enemy actors in sight */
UCLASS( meta = ( BlueprintSpawnableComponent ), ClassGroup = ( Attack ) )
class LORDS_FRONTIERS_API UAttackMeleeComponent : public UAttackComponent
{
	GENERATED_BODY()

public:
	UAttackMeleeComponent();

	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual TObjectPtr<AActor> EnemyInSight() const override;

	virtual void ActivateSight() override;

	virtual void DeactivateSight() override;

protected:
	virtual void BeginPlay() override;

	// Look forward at given time intervals
	void Look();

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	float LookForwardTimeInterval_ = 0.2f;

	UPROPERTY( VisibleAnywhere, Category = "Settings|Attack" )
	TObjectPtr<AActor> EnemyInSight_;

	FTimerHandle SightTimerHandle_;
};
