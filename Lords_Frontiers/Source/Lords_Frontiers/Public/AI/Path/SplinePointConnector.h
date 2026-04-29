#pragma once

#include "ProceduralMeshComponent.h"

#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SplinePointConnector.generated.h"

UCLASS( Abstract )
class LORDS_FRONTIERS_API ASplinePointConnector : public AActor
{
	GENERATED_BODY()

public:
	ASplinePointConnector();

	void AddPoint( FVector worldPosition, bool rebuild = false );
	void RemovePoint( int32 index, bool rebuild = false );
	void BuildSpline();
	void ClearPoints();

	int32 GetPointCount() const
	{
		return SplinePoints_.Num();
	}

private:
	void RebuildMesh();

	// Line width in world units
	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
	float LineWidth_ = 15.f;

	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
	float HeightOffset_ = 0.f;

	// How many samples along the spline — more = smoother
	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
	int32 SampleCount_ = 64;

	// Arrow head length as fraction of total spline length
	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
	float ArrowHeadLength_ = 80.f;

	// Arrow head width
	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
	float ArrowHeadWidth_ = 40.f;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TObjectPtr<UMaterialInterface> LineMaterial_;

	UPROPERTY()
	TObjectPtr<USplineComponent> SplineComponent_;

	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> ProcMesh_;

	UPROPERTY()
	TArray<FVector> SplinePoints_;
};
