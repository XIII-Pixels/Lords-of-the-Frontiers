// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"

#include "FollowComponent.generated.h"

class AUnit;

/** (Gregory-hub)
 * Makes actor chase target */
UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UFollowComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	UFollowComponent();

	void StartFollowing();

	void StopFollowing();

	void SetMaxSpeed( float maxSpeed );

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent( float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;

	void MoveTowardsTarget( float deltaTime );

	void RotateForward( float deltaTime );

	void SnapToGround();

	void Sway( float deltaTime );

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float RotationSpeed_ = 300.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float SnapToGroundDistance_ = 500.0f;

	// Wobble
	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	float SwaySpeed_ = 15.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	float SwayAmplitude_ = 10.0f;

	// Deviate from going forward
	UPROPERTY( EditAnywhere, Category = "Settings|Movement" )
	float MaxDeviationAngle_ = 5.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Movement" )
	float DeviationMaxRate_ = 1.0f;

	float SwayPhaseOffset_ = 0.0f;

	float CurrentSwayRoll_ = 0.0f;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent_;

	UPROPERTY()
	TObjectPtr<AUnit> Unit_;

	FRandomStream StreamRandom_;

	float CurrentDeviationYawSpeed_ = 0.0f;

	float CurrentDeviationYaw_ = 0.0f;

	bool bFollowTarget_ = false;
};
