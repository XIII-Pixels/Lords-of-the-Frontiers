// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Units/Unit.h"
#include "Units/Attack/AttackComponentBase.h"
#include "AttackMeleeComponent.generated.h"

/** (Gregory-hub)
* Makes actor attack enemy actors in sight */
UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class LORDS_FRONTIERS_API UAttackMeleeComponent : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UAttackMeleeComponent();

	virtual void TickComponent(float deltaTime,
		ELevelTick tickType,
		FActorComponentTickFunction* thisTickFunction) override;

	virtual void Attack(TObjectPtr<AActor> hitActor) override;

	virtual TObjectPtr<AActor> EnemyInSight() const override;

protected:
	virtual void BeginPlay() override;

	// Look forward at given time intervals
	void SightTick();

	void LookForward();

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	float LookForwardTimeInterval_ = 0.2f;

	UPROPERTY( VisibleAnywhere, Category = "Settings|Attack" )
	TObjectPtr<AActor> EnemyInSight_;

	UPROPERTY()
	TObjectPtr<AUnit> Unit_;

	FTimerHandle SightTimerHandle_;
};
