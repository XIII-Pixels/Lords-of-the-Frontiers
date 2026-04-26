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

	if ( CloudPlaneMesh && CloudMaterials.Num() > 0 )
	{
		for ( UMaterialInterface* Mat : CloudMaterials )
		{
			UInstancedStaticMeshComponent* ISMC = NewObject<UInstancedStaticMeshComponent>( this );
			ISMC->SetupAttachment( RootComponent );
			ISMC->SetStaticMesh( CloudPlaneMesh );
			ISMC->SetMaterial( 0, Mat );

			ISMC->SetCollisionEnabled( ECollisionEnabled::NoCollision );
			ISMC->SetCastShadow( false );

			ISMC->RegisterComponent();
			CloudISMCs.Add( ISMC );
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
	if ( CloudISMCs.Num() == 0 || CloudSpacing <= 0.0f )
		return;

	FVector Center = GetActorLocation();

	auto SpawnCloud = [&]( FVector Position, FVector OutwardDir )
	{
		Position.X += FMath::RandRange( -RandomOffsetRange, RandomOffsetRange );
		Position.Y += FMath::RandRange( -RandomOffsetRange, RandomOffsetRange );

		float Scale = BaseCloudScale * FMath::RandRange( 0.8f, 1.5f );
		FRotator RandRot = CloudFacingRotation;
		RandRot.Roll += FMath::RandRange( -15.0f, 15.0f );

		FTransform Transform( RandRot, Position, FVector( Scale ) );

		int32 MatIndex = FMath::RandRange( 0, CloudISMCs.Num() - 1 );
		int32 InstIndex = CloudISMCs[MatIndex]->AddInstance( Transform );

		FCloudInstanceData Data;
		Data.ComponentIndex = MatIndex;
		Data.InstanceIndex = InstIndex;
		Data.BaseTransform = Transform;
		Data.OutwardDirection = OutwardDir;
		CloudsData.Add( Data );
	};

	for ( float x = -MapExtents.X; x <= MapExtents.X; x += CloudSpacing )
	{
		SpawnCloud( Center + FVector( x, MapExtents.Y, 0 ), FVector( 0, 1, 0 ) );
		SpawnCloud( Center + FVector( x, -MapExtents.Y, 0 ), FVector( 0, -1, 0 ) );
	}
	for ( float y = -MapExtents.Y; y <= MapExtents.Y; y += CloudSpacing )
	{
		SpawnCloud( Center + FVector( MapExtents.X, y, 0 ), FVector( 1, 0, 0 ) );
		SpawnCloud( Center + FVector( -MapExtents.X, y, 0 ), FVector( -1, 0, 0 ) );
	}
}

void ACloudBorderManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
	if ( !PC || !PC->PlayerCameraManager )
		return;

	UCameraComponent* ActiveCamera = PC->PlayerCameraManager->GetViewTarget()->FindComponentByClass<UCameraComponent>();
	if ( !ActiveCamera )
		return;

	float CurrentZoom = ActiveCamera->GetComponentLocation().Z;

	if ( ActiveCamera->ProjectionMode == ECameraProjectionMode::Orthographic )
	{
		CurrentZoom = ActiveCamera->OrthoWidth;
	}

	float TargetAlpha = FMath::Clamp(
	    ( CurrentZoom - MinZoomThreshold ) / FMath::Max( MaxZoomThreshold - MinZoomThreshold, 1.0f ), 0.0f, 1.0f
	);

	CurrentZoomAlpha = FMath::FInterpTo( CurrentZoomAlpha, TargetAlpha, DeltaTime, TransitionSpeed );

	for ( const FCloudInstanceData& Data : CloudsData )
	{
		float HideFactor = 1.0f - CurrentZoomAlpha;

		FVector NewLocation = Data.BaseTransform.GetLocation();
		NewLocation.Z -= HideOffsetDown * HideFactor;
		NewLocation += Data.OutwardDirection * HideOffsetOutward * HideFactor;

		FTransform NewTransform = Data.BaseTransform;
		NewTransform.SetLocation( NewLocation );
		NewTransform.SetScale3D(
		    Data.BaseTransform.GetScale3D() * FMath::Max( 0.1f, CurrentZoomAlpha )
		);

		CloudISMCs[Data.ComponentIndex]->UpdateInstanceTransform( Data.InstanceIndex, NewTransform, true, false );
	}

	for ( UInstancedStaticMeshComponent* ISMC : CloudISMCs )
	{
		ISMC->MarkRenderStateDirty();
	}
}