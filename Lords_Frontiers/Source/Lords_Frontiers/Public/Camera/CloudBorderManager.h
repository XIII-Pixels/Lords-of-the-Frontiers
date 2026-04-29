#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CloudBorderManager.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;

USTRUCT()
struct FCloudInstanceData
{
	GENERATED_BODY()

	int32 ComponentIndex = 0;
	int32 InstanceIndex = 0;
	FTransform BaseTransform;
	FVector OutwardDirection;
};

UCLASS()
class LORDS_FRONTIERS_API ACloudBorderManager : public AActor
{
	GENERATED_BODY()

public:
	ACloudBorderManager();
	virtual void Tick( float DeltaTime ) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> CloudPlaneMesh;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TArray<TObjectPtr<UMaterialInterface>> CloudMaterials;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visuals" )
	float BaseCloudScale = 5.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visuals" )
	FRotator CloudFacingRotation = FRotator( -50.0f, 45.0f, 0.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation" )
	FVector2D MapExtents = FVector2D( 5000.0f, 5000.0f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation" )
	float CloudSpacing = 800.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation" )
	float RandomOffsetRange = 300.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float MinZoomThreshold = 3000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float MaxZoomThreshold = 8000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float HideOffsetDown = 1500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float HideOffsetOutward = 2000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float TransitionSpeed = 5.0f;

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CloudISMCs;

	TArray<FCloudInstanceData> CloudsData;

	void GenerateClouds();
	float CurrentZoomAlpha = 0.0f;
};