#include "Camera/CloudBorderManager.h"

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ACloudBorderManager::ACloudBorderManager()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );
}

void ACloudBorderManager::BeginPlay()
{
	Super::BeginPlay();

	if ( CloudPlaneMesh && ( CloudMaterials.Num() > 0 ) )
	{
		for ( UMaterialInterface* mat : CloudMaterials )
		{
			UInstancedStaticMeshComponent* ismc = NewObject<UInstancedStaticMeshComponent>( this );
			ismc->SetupAttachment( RootComponent );
			ismc->SetStaticMesh( CloudPlaneMesh );
			ismc->SetMaterial( 0, mat );

			ismc->SetCollisionEnabled( ECollisionEnabled::NoCollision );
			ismc->SetCastShadow( false );

			ismc->RegisterComponent();
			CloudISMCs.Add( ismc );
		}

		GenerateClouds();
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "CloudBorderManager: No Mesh or Materials assigned!" ) );
	}
}

void ACloudBorderManager::GenerateClouds()
{
	if ( ( CloudISMCs.Num() == 0 ) || ( CloudSpacing <= 0.0f ) )
	{
		return;
	}

	FVector center = FVector::ZeroVector;

	FRandomStream rng( 12345 );

	auto spawnCloud = [&]( FVector position, FVector outwardDir )
	{
		position.X += rng.FRandRange( -RandomOffsetRange, RandomOffsetRange );
		position.Y += rng.FRandRange( -RandomOffsetRange, RandomOffsetRange );

		float scale = BaseCloudScale * rng.FRandRange( 0.8f, 1.5f );
		FRotator randRot = CloudFacingRotation;
		randRot.Roll += rng.FRandRange( -15.0f, 15.0f );

		FTransform transform( randRot, position, FVector( scale ) );

		int32 matIndex = rng.RandRange( 0, CloudISMCs.Num() - 1 );
		int32 instIndex = CloudISMCs[matIndex]->AddInstance( transform );

		FCloudInstanceData data;
		data.ComponentIndex = matIndex;
		data.InstanceIndex = instIndex;
		data.BaseTransform = transform;
		data.OutwardDirection = outwardDir;

		CloudsData.Add( data );
	};

	int32 numLayers = 4;

	for ( int32 layer = 0; layer < numLayers; ++layer )
	{
		float currentRx = MapExtents.X + ( layer * CloudSpacing * 0.8f );
		float currentRy = MapExtents.Y + ( layer * CloudSpacing * 0.8f );

		float perimeter = 2.0f * PI * FMath::Sqrt( ( currentRx * currentRx + currentRy * currentRy ) / 2.0f );
		int32 cloudsInThisLayer = FMath::CeilToInt( perimeter / CloudSpacing );

		for ( int32 i = 0; i < cloudsInThisLayer; ++i )
		{
			float angle = ( (float) i / (float) cloudsInThisLayer ) * 2.0f * PI;

			float x = currentRx * FMath::Cos( angle );
			float y = currentRy * FMath::Sin( angle );

			FVector spawnPos = center + FVector( x, y, 0.0f );
			FVector outwardDir = FVector( x, y, 0.0f ).GetSafeNormal();

			spawnCloud( spawnPos, outwardDir );
		}
	}
}

void ACloudBorderManager::Tick( float deltaTime )
{
	Super::Tick( deltaTime );

	APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 );

	if ( !IsValid( pc ) || !IsValid( pc->PlayerCameraManager ) )
	{
		return;
	}

	UCameraComponent* activeCamera = pc->PlayerCameraManager->GetViewTarget()->FindComponentByClass<UCameraComponent>();

	if ( !IsValid( activeCamera ) )
	{
		return;
	}

	float currentZoom = activeCamera->GetComponentLocation().Z;

	if ( activeCamera->ProjectionMode == ECameraProjectionMode::Orthographic )
	{
		currentZoom = activeCamera->OrthoWidth;
	}

	float targetAlpha = FMath::Clamp(
	    ( currentZoom - MinZoomThreshold ) / FMath::Max( MaxZoomThreshold - MinZoomThreshold, 1.0f ), 0.0f, 1.0f
	);

	CurrentZoomAlpha = FMath::FInterpTo( CurrentZoomAlpha, targetAlpha, deltaTime, TransitionSpeed );

	FRotator camRot = activeCamera->GetComponentRotation();
	FRotator faceCameraRot = camRot;
	faceCameraRot.Pitch += 90.0f;
	FQuat localRot = GetActorTransform().InverseTransformRotation( faceCameraRot.Quaternion() );

	for ( const FCloudInstanceData& data : CloudsData )
	{
		float hideFactor = 1.0f - CurrentZoomAlpha;

		FVector newLocation = data.BaseTransform.GetLocation();
		newLocation.Z -= HideOffsetDown * hideFactor;
		newLocation += data.OutwardDirection * HideOffsetOutward * hideFactor;

		FTransform newTransform = data.BaseTransform;
		newTransform.SetLocation( newLocation );

		FRotator finalRot = localRot.Rotator();
		finalRot.Roll += data.BaseTransform.Rotator().Roll;
		newTransform.SetRotation( finalRot.Quaternion() );

		newTransform.SetScale3D( data.BaseTransform.GetScale3D() * FMath::Max( 0.1f, CurrentZoomAlpha ) );

		if ( IsValid( CloudISMCs[data.ComponentIndex] ) )
		{
			CloudISMCs[data.ComponentIndex]->UpdateInstanceTransform( data.InstanceIndex, newTransform, false, false );
		}
	}

	for ( UInstancedStaticMeshComponent* ismc : CloudISMCs )
	{
		if ( IsValid( ismc ) )
		{
			ismc->MarkRenderStateDirty();
		}
	}
}