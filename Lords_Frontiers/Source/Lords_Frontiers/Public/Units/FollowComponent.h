// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "FollowComponent.generated.h"

/** (Gregory-hub)
* Makes actor chase target actor */
UCLASS( ClassGroup=(Movement), meta=(BlueprintSpawnableComponent) )
class LORDS_FRONTIERS_API UFollowComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	UFollowComponent();

	const TObjectPtr<AActor>& Target() const;

	void StartFollowing();
	
	void StopFollowing();

protected:
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float deltaTime,
		ELevelTick tickType,
		FActorComponentTickFunction* thisTickFunction) override;

	void MoveTowardsTarget(float deltaTime);

	void RotateForward(float deltaTime);
	
	void SnapToNavMeshGround();

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float MaxSpeed_ = 300.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float RotationSpeed_ = 300.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Movement" )
	float SnapToGroundDistance_ = 500.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Movement" )
	TObjectPtr<AActor> FollowedTarget_;

	bool bFollowTarget = false;
};
