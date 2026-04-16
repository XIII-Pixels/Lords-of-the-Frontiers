// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"

#include "StrategyCamera.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UInputComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class LORDS_FRONTIERS_API AStrategyCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AStrategyCamera();

	// components
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> RotateAction; // Q/E (Axis1D)

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> PauseAction; //ESC

	UFUNCTION( BlueprintCallable, Category = "Settings|Input" )
	void SetCameraInputDisabled( bool bDisabled )
	{
		bIsCameraInputDisabled_ = bDisabled;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Input" )
	bool IsCameraInputDisabled() const
	{
		return bIsCameraInputDisabled_;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// wasd
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float MoveSpeed_ = 2000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float MoveAcceleration_ = 8000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float MoveDeceleration_ = 4000.0f;

	// zoom
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float ZoomSpeed_ = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float MinZoom_ = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float MaxZoom_ = 5000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float ZoomInterpSpeed_ = 10.0f;

	// izometric
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visual" )
	float CameraPitch_ = -50.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visual" )
	float CameraYaw_ = 45.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visual" )
	float CameraLagSpeed_ = 5.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bounds" )
	FVector2D MinMapBounds_ = FVector2D( 0.0f, 0.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bounds" )
	FVector2D MaxMapBounds_ = FVector2D( 5000.0f, 5000.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bounds" )
	bool bAutoCalculateBounds_ = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bounds" )
	FVector2D MinMoveAreaExtents_ = FVector2D( 0.0f, 0.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bounds", meta = ( ClampMin = "0.1" ) )
	float BoundsCurveExponent_ = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visual" )
	float RotationSpeed_ = 45.0f;

	//Edge Scrolling
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	bool bEnableEdgeScrolling_ = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float EdgeScrollSpeed_ = 2000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float EdgeScrollThreshold_ = 20.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|CameraType" )
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionMode_ = ECameraProjectionMode::Orthographic;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|CameraType",
	    meta = ( EditCondition = "ProjectionMode_ == ECameraProjectionMode::Perspective" )
	)
	float FieldOfView_ = 15.0f; //fake orto

	//orto
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom",
	    meta = ( EditCondition = "ProjectionMode_ == ECameraProjectionMode::Orthographic" )
	)
	float InitialOrthoWidth_ = 2048.0f;

	//perspectiva
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom",
	    meta = ( EditCondition = "ProjectionMode_ == ECameraProjectionMode::Perspective" )
	)
	float InitialTargetArmLength_ = 8000.0f;

public:	
	// Called every frame
	virtual void Tick(float deltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;

private:
	void Move( const FInputActionValue& value );
	void Zoom( const FInputActionValue& value );

	void Rotate( const FInputActionValue& value );
	void HandleEdgeScrolling();
	void TogglePause( const FInputActionValue& value );

	float TargetZoom_;
	float TargetYaw_;
	float CurrentYaw_;
	bool bIsCameraInputDisabled_ = false;

	FVector2D MapCenter_;
	FVector2D MaxMoveAreaExtents_;
};
