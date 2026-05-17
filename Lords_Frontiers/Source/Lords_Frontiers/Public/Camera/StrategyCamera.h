// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputActionValue.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "StrategyCamera.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class LORDS_FRONTIERS_API AStrategyCamera : public APawn
{
	GENERATED_BODY()

public:
	AStrategyCamera();

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<USceneComponent> AudioListener;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Input" )
	TObjectPtr<UInputAction> PauseAction;

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

	UFUNCTION( BlueprintCallable, Category = "Settings|Zoom" )
	void SetZoomToMax();

	UFUNCTION( BlueprintCallable, Category = "Settings|Zoom" )
	void SetZoomDisabled( bool bDisabled )
	{
		bIsZoomDisabled_ = bDisabled;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Zoom" )
	bool IsZoomDisabled() const
	{
		return bIsZoomDisabled_;
	}

	UFUNCTION( BlueprintCallable, Category = "Settings|Bounds" )
	void SetIgnoreZoomBoundsCurve( bool bIgnore )
	{
		bIgnoreZoomBoundsCurve_ = bIgnore;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Bounds" )
	bool IsIgnoringZoomBoundsCurve() const
	{
		return bIgnoreZoomBoundsCurve_;
	}

	float MinZoom() const
	{
		return MinZoom_;
	}

	float MaxZoom() const
	{
		return MaxZoom_;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float MoveSpeed_ = 2000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float MoveAcceleration_ = 8000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	float MoveDeceleration_ = 4000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float ZoomSpeed_ = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float MinZoom_ = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float MaxZoom_ = 5000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom" )
	float ZoomInterpSpeed_ = 10.0f;

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
	float FieldOfView_ = 15.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom",
	    meta = ( EditCondition = "ProjectionMode_ == ECameraProjectionMode::Orthographic" )
	)
	float InitialOrthoWidth_ = 2048.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Zoom",
	    meta = ( EditCondition = "ProjectionMode_ == ECameraProjectionMode::Perspective" )
	)
	float InitialTargetArmLength_ = 8000.0f;

public:
	virtual void Tick( float deltaTime ) override;
	virtual void SetupPlayerInputComponent( class UInputComponent* playerInputComponent ) override;

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
	bool bIsZoomDisabled_ = false;
	bool bIgnoreZoomBoundsCurve_ = false;

	FVector2D MapCenter_;
	FVector2D MaxMoveAreaExtents_;

	FVector CurrentVelocity_ = FVector::ZeroVector;
};
