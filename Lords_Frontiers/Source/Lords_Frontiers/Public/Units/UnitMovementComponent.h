// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "UnitMovementComponent.generated.h"

/** (Gregory-hub)
* Class responsible for unit movement */
UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class LORDS_FRONTIERS_API UUnitMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	UUnitMovementComponent();

	void TickComponent(float deltaTime,
		ELevelTick tickType,
		FActorComponentTickFunction* thisTickFunction) override;

	void BeginPlay() override;

	UPROPERTY( EditAnywhere, Category = "Movement" )
	float SnapToGroundDistance = 100.0f;

protected:
	void SnapToNavMeshGround();

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent_;
};
