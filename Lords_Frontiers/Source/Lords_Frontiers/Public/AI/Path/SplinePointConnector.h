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
    void RemovePoint( int32 index );
    void BuildSpline();
    void Clear();

    int32 GetPointCount() const { return SplinePoints_.Num(); }

private:
	virtual void BeginPlay() override;

    void RebuildMesh();
    void SampleLineVertices( float fromDist, float toDist, int32 samples,
                             TArray<FVector>& vertices, TArray<FVector>& normals,
                             TArray<FVector2D>& uvs, float totalLength ) const;
    void BuildTriangles( int32 vertCount, TArray<int32>& triangles ) const;

    UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
    float LineWidth_ = 15.f;

    UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
    float HeightOffset_ = 0.f;

    UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 1 ) )
    int32 SampleCount_ = 64;

    UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
    float ArrowHeadLength_ = 80.f;

    UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
    float ArrowHeadWidth_ = 40.f;

	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0 ) )
	float ArrowOffset_ = 0.f;

    UPROPERTY( EditAnywhere, Category = "Settings" )
    TObjectPtr<UMaterialInterface> LineMaterial_;

    UPROPERTY()
    TObjectPtr<USplineComponent> SplineComponent_;

    UPROPERTY()
    TObjectPtr<UProceduralMeshComponent> ProcMesh_;

    UPROPERTY()
    TArray<FVector> SplinePoints_;

	UPROPERTY()
	float Height_ = 0.0f;
};
