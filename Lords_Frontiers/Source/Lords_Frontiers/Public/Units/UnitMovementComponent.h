// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "UnitMovementComponent.generated.h"

/** (Gregory-hub)
* Class responsible for unit movement */
UCLASS( meta=(BlueprintSpawnableComponent) )
class LORDS_FRONTIERS_API UUnitMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	UUnitMovementComponent();

	void TickComponent(float deltaTime,
		ELevelTick tickType,
		FActorComponentTickFunction* thisTickFunction) override;

	void BeginPlay() override;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float MovementSpeed = 800.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float RotationSpeed = 500.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float SnapToGroundDistance = 100.0f;

protected:
	void SnapToNavMeshGround();

	void RotateForward(float deltaTime);

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent_;
};
