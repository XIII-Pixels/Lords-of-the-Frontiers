// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "StrategyCamera.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UInputComponent;

UCLASS()
class LORDS_FRONTIERS_API AStrategyCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AStrategyCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// components
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Settings|Components" )
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Settings|Components" )
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Settings|Components" )
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	// wasd
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Movement" )
	float MoveSpeed_ = 2000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Movement" )
	float MoveAcceleration_ = 8000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Movement" )
	float MoveDeceleration_ = 4000.0f;

	// zoom
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Zoom" )
	float ZoomSpeed_ = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Zoom" )
	float MinZoom_ = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Zoom" )
	float MaxZoom_ = 5000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Zoom" )
	float ZoomInterpSpeed_ = 10.0f;

	// izometric
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Visual" )
	float CameraPitch_ = -50.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Visual" )
	float CameraYaw_ = 45.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Visual" )
	float CameraLagSpeed_ = 5.0f;

	// map restrictions
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera|Settings|Bounds" )
	FVector2D MapBounds_ = FVector2D( 10000.0f, 10000.0f );


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;

private:
	void MoveForward(float value);
	void MoveRight(float value);
	void ZoomIn();
	void ZoomOut();

	float TargetZoom_;
};
