// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "UnitMovementComponent.generated.h"

UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class LORDS_FRONTIERS_API UUnitMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	void TickComponent(float deltaTime,
		ELevelTick tickType,
		FActorComponentTickFunction* thisTickFunction) override;

	void BeginPlay() override;

	UPROPERTY( EditDefaultsOnly, Category = "Movement" )
	float SnapToGroundDistance = 500.0f;

protected:
	void SnapToNavMeshGround();

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent_;
};
