// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/StrategyCamera.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AStrategyCamera::AStrategyCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );

	SpringArm = CreateDefaultSubobject<USpringArmComponent>( TEXT( "SpringArm" ) );
	SpringArm->SetupAttachment( RootComponent );
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>( TEXT( "Camera" ) );
	Camera->SetupAttachment( SpringArm );
	Camera->bUsePawnControlRotation = false;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>( TEXT( "MovementComponent" ) );

	TargetZoom = 1500.0f;

}

// Called when the game starts or when spawned
void AStrategyCamera::BeginPlay()
{
	Super::BeginPlay();

	FVector StartingLocation = GetActorLocation();
	StartingLocation.Z = 0.0f;
	SetActorLocation( StartingLocation );

	MovementComponent->MaxSpeed = MoveSpeed;
	MovementComponent->Acceleration = MoveAcceleration;
	MovementComponent->Deceleration = MoveDeceleration;

	SpringArm->TargetArmLength = TargetZoom;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->SetRelativeRotation( FRotator( CameraPitch, CameraYaw, 0.0f ) );

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ACameraActor::StaticClass(), FoundCameras );
	for ( AActor* Cam : FoundCameras )
	{
		Cam->Destroy();
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
	    TimerHandle,
	    [this]()
	    {
		    if ( APlayerController* pc = Cast<APlayerController>( GetController() ) )
		    {
			    pc->SetViewTarget( this );
			    pc->bShowMouseCursor = true;
			    pc->SetInputMode( FInputModeGameAndUI() );
		    }
	    },
	    0.2f, false
	);
	
}

// Called every frame
void AStrategyCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo( SpringArm->TargetArmLength, TargetZoom, DeltaTime, ZoomInterpSpeed );

	FVector ClampedLocation = GetActorLocation();
	ClampedLocation.X = FMath::Clamp( ClampedLocation.X, -MapBounds.X, MapBounds.X );
	ClampedLocation.Y = FMath::Clamp( ClampedLocation.Y, -MapBounds.Y, MapBounds.Y );
	SetActorLocation( ClampedLocation );

}

// Called to bind functionality to input
void AStrategyCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis( "MoveForward", this, &AStrategyCamera::MoveForward );
	PlayerInputComponent->BindAxis( "MoveRight", this, &AStrategyCamera::MoveRight );

	PlayerInputComponent->BindAction( "ZoomIn", IE_Pressed, this, &AStrategyCamera::ZoomIn );
	PlayerInputComponent->BindAction( "ZoomOut", IE_Pressed, this, &AStrategyCamera::ZoomOut );

}

void AStrategyCamera::MoveForward( float value )
{
	AddMovementInput( GetActorForwardVector(), value );
}
void AStrategyCamera::MoveRight( float value )
{
	AddMovementInput( GetActorRightVector(), value );
}

void AStrategyCamera::ZoomIn()
{
	TargetZoom = FMath::Clamp( TargetZoom - ZoomSpeed, MinZoom, MaxZoom );
}
void AStrategyCamera::ZoomOut()
{
	TargetZoom = FMath::Clamp( TargetZoom + ZoomSpeed, MinZoom, MaxZoom );
}