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

	FVector Center = FVector::ZeroVector;

	FRandomStream RNG( 12345 );

	auto SpawnCloud = [&]( FVector Position, FVector OutwardDir )
	{
		Position.X += RNG.FRandRange( -RandomOffsetRange, RandomOffsetRange );
		Position.Y += RNG.FRandRange( -RandomOffsetRange, RandomOffsetRange );

		float Scale = BaseCloudScale * RNG.FRandRange( 0.8f, 1.5f );
		FRotator RandRot = CloudFacingRotation;
		RandRot.Roll += RNG.FRandRange( -15.0f, 15.0f );

		FTransform Transform( RandRot, Position, FVector( Scale ) );

		int32 MatIndex = RNG.RandRange( 0, CloudISMCs.Num() - 1 );
		int32 InstIndex = CloudISMCs[MatIndex]->AddInstance( Transform );

		FCloudInstanceData Data;
		Data.ComponentIndex = MatIndex;
		Data.InstanceIndex = InstIndex;
		Data.BaseTransform = Transform;
		Data.OutwardDirection = OutwardDir;
		CloudsData.Add( Data );
	};

	int32 NumLayers = 4;

	for ( int32 Layer = 0; Layer < NumLayers; ++Layer )
	{
		float CurrentRx = MapExtents.X + ( Layer * CloudSpacing * 0.8f );
		float CurrentRy = MapExtents.Y + ( Layer * CloudSpacing * 0.8f );

		float Perimeter = 2.0f * PI * FMath::Sqrt( ( CurrentRx * CurrentRx + CurrentRy * CurrentRy ) / 2.0f );
		int32 CloudsInThisLayer = FMath::CeilToInt( Perimeter / CloudSpacing );

		for ( int32 i = 0; i < CloudsInThisLayer; ++i )
		{
			float Angle = ( (float) i / (float) CloudsInThisLayer ) * 2.0f * PI;

			float X = CurrentRx * FMath::Cos( Angle );
			float Y = CurrentRy * FMath::Sin( Angle );

			FVector SpawnPos = Center + FVector( X, Y, 0.0f );
			FVector OutwardDir = FVector( X, Y, 0.0f ).GetSafeNormal();

			SpawnCloud( SpawnPos, OutwardDir );
		}
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

	FRotator CamRot = ActiveCamera->GetComponentRotation();
	FRotator FaceCameraRot = CamRot;
	FaceCameraRot.Pitch += 90.0f;
	FQuat LocalRot = GetActorTransform().InverseTransformRotation( FaceCameraRot.Quaternion() );

	for ( const FCloudInstanceData& Data : CloudsData )
	{
		float HideFactor = 1.0f - CurrentZoomAlpha;

		FVector NewLocation = Data.BaseTransform.GetLocation();
		NewLocation.Z -= HideOffsetDown * HideFactor;
		NewLocation += Data.OutwardDirection * HideOffsetOutward * HideFactor;

		FTransform NewTransform = Data.BaseTransform;
		NewTransform.SetLocation( NewLocation );

		FRotator FinalRot = LocalRot.Rotator();
		FinalRot.Roll += Data.BaseTransform.Rotator().Roll;
		NewTransform.SetRotation( FinalRot.Quaternion() );

		NewTransform.SetScale3D( Data.BaseTransform.GetScale3D() * FMath::Max( 0.1f, CurrentZoomAlpha ) );

		CloudISMCs[Data.ComponentIndex]->UpdateInstanceTransform( Data.InstanceIndex, NewTransform, false, false );
	}

	for ( UInstancedStaticMeshComponent* ISMC : CloudISMCs )
	{
		ISMC->MarkRenderStateDirty();
	}
}