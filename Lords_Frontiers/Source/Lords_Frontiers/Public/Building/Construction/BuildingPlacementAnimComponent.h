// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "BuildingPlacementAnimComponent.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

USTRUCT( BlueprintType )
struct FBuildPlacementAnimParams
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float Duration = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float ScaleMultiplier = 1.3f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float JumpHeight = 30.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settins|Animation" )
	float EaseExponent = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	FLinearColor FlashColor = FLinearColor( 3.0f, 3.0f, 3.0f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	FName EmissiveParamName = TEXT( "EmissiveColor" );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float Phase1EndParams = 0.35f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float Phase2EndParams = 0.65f;
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UBuildingPlacementAnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingPlacementAnimComponent();

	void StartAnimation( const FBuildPlacementAnimParams& inParams );

protected:
	virtual void
	TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

private:
	void FinishAnimation();

	void
	EvaluateAppearPhase( float totalAlpha, FVector& outLocation, FVector& outScale, float& outFlashIntensity ) const;

	void EvaluateJumpPhase( float totalAlpha, FVector& outLocation ) const;

	void EvaluateLandPhase( float totalAlpha, FVector& outScale, FVector& outLocation ) const;

	void ApplyFlashColor( float flashIntensity );

	FBuildPlacementAnimParams Params_;

	FVector OriginalLocation_;
	FVector OriginalScale_;
	float Elapsed_ = 0.0f;
	bool bIsAnimating_ = false;
	float Phase1End_ = 0.35f;
	float Phase2End_ = 0.65f;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh_;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials_;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials_;
};
