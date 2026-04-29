#include "AI/Path/SplinePointConnector.h"

#include "Components/SplineComponent.h"

ASplinePointConnector::ASplinePointConnector()
{
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent_ = CreateDefaultSubobject<USplineComponent>( TEXT( "SplineComponent" ) );
	SplineComponent_->ClearSplinePoints();
	SetRootComponent( SplineComponent_ );

	ProcMesh_ = CreateDefaultSubobject<UProceduralMeshComponent>( TEXT( "ProcMesh" ) );
	ProcMesh_->AttachToComponent( SplineComponent_, FAttachmentTransformRules::KeepRelativeTransform );
	ProcMesh_->SetCastShadow( false );
}

void ASplinePointConnector::AddPoint( FVector worldPosition, bool rebuild )
{
	if ( !IsValid( SplineComponent_ ) )
	{
		return;
	}
	SplinePoints_.Add( worldPosition );
	if ( rebuild )
	{
		BuildSpline();
	}
}

void ASplinePointConnector::RemovePoint( int32 index )
{
	if ( !SplinePoints_.IsValidIndex( index ) )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "ASplinePointConnector::RemovePoint — Index %d out of range (%d points)" ), index,
		    SplinePoints_.Num()
		);
		return;
	}

	USplineComponent* spline = SplineComponent_.Get();
	FVector arrivePrev, leavePrev, arriveNext, leaveNext;
	const bool bHasPrev = SplinePoints_.IsValidIndex( index - 1 );
	const bool bHasNext = SplinePoints_.IsValidIndex( index + 1 );

	if ( bHasPrev )
	{
		arrivePrev = spline->GetArriveTangentAtSplinePoint( index - 1, ESplineCoordinateSpace::World );
		leavePrev = spline->GetLeaveTangentAtSplinePoint( index - 1, ESplineCoordinateSpace::World );
	}
	if ( bHasNext )
	{
		arriveNext = spline->GetArriveTangentAtSplinePoint( index + 1, ESplineCoordinateSpace::World );
		leaveNext = spline->GetLeaveTangentAtSplinePoint( index + 1, ESplineCoordinateSpace::World );
	}

	SplinePoints_.RemoveAt( index );
	BuildSpline();

	if ( bHasPrev && SplinePoints_.IsValidIndex( index - 1 ) )
	{
		spline->SetTangentsAtSplinePoint( index - 1, arrivePrev, leavePrev, ESplineCoordinateSpace::World );
	}
	if ( bHasNext && SplinePoints_.IsValidIndex( index ) )
	{
		spline->SetTangentsAtSplinePoint( index, arriveNext, leaveNext, ESplineCoordinateSpace::World );
	}

	spline->UpdateSpline();
	RebuildMesh();
}

void ASplinePointConnector::BuildSpline()
{
	USplineComponent* spline = SplineComponent_.Get();
	spline->ClearSplinePoints( false );

	const FTransform& t = spline->GetComponentTransform();
	for ( int32 i = 0; i < SplinePoints_.Num(); i++ )
	{
		FSplinePoint p;
		p.InputKey = static_cast<float>( i );
		p.Position = t.InverseTransformPosition( SplinePoints_[i] );
		p.Type = ESplinePointType::Curve;
		spline->AddPoint( p, false );
	}
	spline->UpdateSpline();
	RebuildMesh();
}

void ASplinePointConnector::Clear()
{
	SplinePoints_.Empty();
	SplineComponent_->ClearSplinePoints();
	ProcMesh_->ClearAllMeshSections();
}

void ASplinePointConnector::RebuildMesh()
{
	ProcMesh_->ClearAllMeshSections();
	if ( SplinePoints_.Num() < 2 )
	{
		return;
	}

	USplineComponent* spline = SplineComponent_.Get();
	const float totalLength = spline->GetSplineLength();
	const float arrowStartDist = FMath::Max( totalLength - ArrowHeadLength_, totalLength * 0.5f );

	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FVector2D> uvs;

	SampleLineVertices( 0.f, arrowStartDist, SampleCount_, vertices, normals, uvs, totalLength );
	BuildTriangles( vertices.Num(), triangles );

	const FVector tipCenter = spline->GetLocationAtDistanceAlongSpline( totalLength, ESplineCoordinateSpace::World ) +
	                          FVector( 0.f, 0.f, HeightOffset_ );
	const FVector baseCenter =
	    spline->GetLocationAtDistanceAlongSpline( arrowStartDist, ESplineCoordinateSpace::World ) +
	    FVector( 0.f, 0.f, HeightOffset_ );
	const FVector baseTangent =
	    spline->GetTangentAtDistanceAlongSpline( arrowStartDist, ESplineCoordinateSpace::World ).GetSafeNormal();
	const FVector baseRight = FVector::CrossProduct( baseTangent, FVector::UpVector ).GetSafeNormal();

	const int32 arrowBase = vertices.Num();
	vertices.Add( baseCenter - baseRight * ArrowHeadWidth_ );
	vertices.Add( baseCenter + baseRight * ArrowHeadWidth_ );
	vertices.Add( tipCenter );
	normals.Add( FVector::UpVector );
	normals.Add( FVector::UpVector );
	normals.Add( FVector::UpVector );
	uvs.Add( FVector2D( 0.f, 0.f ) );
	uvs.Add( FVector2D( 1.f, 0.f ) );
	uvs.Add( FVector2D( 0.5f, 1.f ) );
	triangles.Add( arrowBase );
	triangles.Add( arrowBase + 2 );
	triangles.Add( arrowBase + 1 );

	ProcMesh_->CreateMeshSection(
	    0, vertices, triangles, normals, uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false
	);
	if ( IsValid( LineMaterial_ ) )
	{
		ProcMesh_->SetMaterial( 0, LineMaterial_ );
	}
}

void ASplinePointConnector::SampleLineVertices(
    float fromDist, float toDist, int32 samples, TArray<FVector>& vertices, TArray<FVector>& normals,
    TArray<FVector2D>& uvs, float totalLength
) const
{
	USplineComponent* spline = SplineComponent_.Get();
	const float clampedFrom = FMath::Clamp( fromDist, 0.f, totalLength );
	const float clampedTo = FMath::Clamp( toDist, 0.f, totalLength );

	for ( int32 i = 0; i <= samples; i++ )
	{
		const float distance = FMath::Lerp( clampedFrom, clampedTo, static_cast<float>( i ) / samples );

		const FVector center = spline->GetLocationAtDistanceAlongSpline( distance, ESplineCoordinateSpace::World ) +
		                       FVector( 0.f, 0.f, HeightOffset_ );
		const FVector tangent =
		    spline->GetTangentAtDistanceAlongSpline( distance, ESplineCoordinateSpace::World ).GetSafeNormal();
		const FVector right = FVector::CrossProduct( tangent, FVector::UpVector ).GetSafeNormal();
		const float u = totalLength > 0.f ? distance / totalLength : 0.f;

		vertices.Add( center - right * ( LineWidth_ * 0.5f ) );
		vertices.Add( center + right * ( LineWidth_ * 0.5f ) );
		normals.Add( FVector::UpVector );
		normals.Add( FVector::UpVector );
		uvs.Add( FVector2D( 0.f, u ) );
		uvs.Add( FVector2D( 1.f, u ) );
	}
}

void ASplinePointConnector::BuildTriangles( int32 vertCount, TArray<int32>& triangles ) const
{
	for ( int32 i = 0; i < vertCount - 2; i += 2 )
	{
		triangles.Add( i );
		triangles.Add( i + 2 );
		triangles.Add( i + 1 );
		triangles.Add( i + 1 );
		triangles.Add( i + 2 );
		triangles.Add( i + 3 );
	}
}
