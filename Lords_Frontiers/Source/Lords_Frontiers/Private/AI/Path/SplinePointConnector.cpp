#include "AI/Path/SplinePointConnector.h"

#include "Components/SplineComponent.h"
#include "KismetProceduralMeshLibrary.h"

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

void ASplinePointConnector::RemovePoint( int32 index, bool rebuild )
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

	if ( rebuild )
	{
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
}

void ASplinePointConnector::BuildSpline()
{
	USplineComponent* spline = SplineComponent_.Get();
	spline->ClearSplinePoints( false );

	const FTransform& splineTransform = spline->GetComponentTransform();

	for ( int32 i = 0; i < SplinePoints_.Num(); i++ )
	{
		FSplinePoint newPoint;
		newPoint.InputKey = static_cast<float>( i );
		newPoint.Position = splineTransform.InverseTransformPosition( SplinePoints_[i] );
		newPoint.Type = ESplinePointType::Curve;
		spline->AddPoint( newPoint, false );
	}

	spline->UpdateSpline();
	RebuildMesh();
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
	TArray<FColor> colors;

	// ── Line body (quad strip) ──────────────────────────────────────────
	for ( int32 i = 0; i <= SampleCount_; i++ )
	{
		const float distance = totalLength * ( static_cast<float>( i ) / SampleCount_ );
		if ( distance > arrowStartDist )
		{
			break;
		}

		const FVector center = spline->GetLocationAtDistanceAlongSpline( distance, ESplineCoordinateSpace::World ) +
		                       FVector( 0.f, 0.f, HeightOffset_ );
		const FVector tangent =
		    spline->GetTangentAtDistanceAlongSpline( distance, ESplineCoordinateSpace::World ).GetSafeNormal();
		const FVector up = FVector::UpVector;
		const FVector right = FVector::CrossProduct( tangent, up ).GetSafeNormal();

		const float u = static_cast<float>( i ) / SampleCount_;

		vertices.Add( center - right * ( LineWidth_ * 0.5f ) );
		vertices.Add( center + right * ( LineWidth_ * 0.5f ) );

		normals.Add( up );
		normals.Add( up );

		uvs.Add( FVector2D( 0.f, u ) );
		uvs.Add( FVector2D( 1.f, u ) );
	}

	// Build quads for the body
	const int32 bodyVerts = vertices.Num();
	for ( int32 i = 0; i < bodyVerts - 2; i += 2 )
	{
		// Triangle 1
		triangles.Add( i );
		triangles.Add( i + 2 );
		triangles.Add( i + 1 );
		// Triangle 2
		triangles.Add( i + 1 );
		triangles.Add( i + 2 );
		triangles.Add( i + 3 );
	}

	// ── Arrow head (triangle) ───────────────────────────────────────────
	const FVector tipCenter = spline->GetLocationAtDistanceAlongSpline( totalLength, ESplineCoordinateSpace::World ) +
	                          FVector( 0.f, 0.f, HeightOffset_ );
	const FVector baseCenter =
	    spline->GetLocationAtDistanceAlongSpline( arrowStartDist, ESplineCoordinateSpace::World ) +
	    FVector( 0.f, 0.f, HeightOffset_ );
	const FVector baseTangent =
	    spline->GetTangentAtDistanceAlongSpline( arrowStartDist, ESplineCoordinateSpace::World ).GetSafeNormal();
	const FVector baseRight = FVector::CrossProduct( baseTangent, FVector::UpVector ).GetSafeNormal();

	const int32 arrowBase = vertices.Num();

	vertices.Add( baseCenter - baseRight * ArrowHeadWidth_ );	// left base
	vertices.Add( baseCenter + baseRight * ArrowHeadWidth_ );	// right base
	vertices.Add( tipCenter );										// tip

	normals.Add( FVector::UpVector );
	normals.Add( FVector::UpVector );
	normals.Add( FVector::UpVector );

	uvs.Add( FVector2D( 0.f, 0.f ) );
	uvs.Add( FVector2D( 1.f, 0.f ) );
	uvs.Add( FVector2D( 0.5f, 1.f ) );

	triangles.Add( arrowBase );
	triangles.Add( arrowBase + 2 );
	triangles.Add( arrowBase + 1 );

	// ── Create mesh section ─────────────────────────────────────────────
	ProcMesh_->CreateMeshSection(
	    0, vertices, triangles, normals, uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false
	);

	if ( IsValid( LineMaterial_ ) )
	{
		ProcMesh_->SetMaterial( 0, LineMaterial_ );
	}
}

void ASplinePointConnector::ClearPoints()
{
	SplinePoints_.Empty();
	SplineComponent_->ClearSplinePoints();
	ProcMesh_->ClearAllMeshSections();
}
