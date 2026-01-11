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

	virtual void
	TickComponent( float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;

	void MoveTowardsTarget();

	void RotateForward( float deltaTime );

	void SnapToNavMeshGround();

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float RotationSpeed_ = 300.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float SnapToGroundDistance_ = 500.0f;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent_;

	UPROPERTY()
	TObjectPtr<AUnit> Unit_;

	bool bFollowTarget_ = false;
};
