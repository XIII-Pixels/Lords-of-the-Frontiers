// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/StrategyCamera.h"
#include "Grid/GridManager.h"

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

	TargetZoom_ = 1500.0f;

}

// Called when the game starts or when spawned
void AStrategyCamera::BeginPlay()
{
	Super::BeginPlay();

	FVector startingLocation = GetActorLocation();
	startingLocation.Z = 0.0f;
	SetActorLocation( startingLocation );

	MovementComponent->MaxSpeed = MoveSpeed_;
	MovementComponent->Acceleration = MoveAcceleration_;
	MovementComponent->Deceleration = MoveDeceleration_;

	SpringArm->TargetArmLength = TargetZoom_;
	SpringArm->CameraLagSpeed = CameraLagSpeed_;
	SpringArm->SetRelativeRotation( FRotator( CameraPitch_, CameraYaw_, 0.0f ) );

	TArray<AActor*> foundCameras;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ACameraActor::StaticClass(), foundCameras );
	for ( AActor* cam : foundCameras )
	{
		cam->Destroy();
	}

	if ( bAutoCalculateBounds_ )
	{
		if ( AGridManager* grid =
		         Cast<AGridManager>( UGameplayStatics::GetActorOfClass( GetWorld(), AGridManager::StaticClass() ) ) )
		{
			FVector gridOrigin = grid->GetActorLocation();
			float gridWidth = grid->GetGridWidth() * grid->GetCellSize();
			float gridHeight = grid->GetGridHeight() * grid->GetCellSize();

			MinMapBounds_ = FVector2D( gridOrigin.X, gridOrigin.Y );
			MaxMapBounds_ = FVector2D( gridOrigin.X + gridWidth, gridOrigin.Y + gridHeight );

			UE_LOG(
			    LogTemp, Warning, TEXT( "Camera: Bounds set from Grid! Min: %s, Max: %s" ), *MinMapBounds_.ToString(),
			    *MaxMapBounds_.ToString()
			);
		}
	}

	FTimerHandle timerHandle;
	GetWorldTimerManager().SetTimer(
	    timerHandle,
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

	SpringArm->TargetArmLength = FMath::FInterpTo( SpringArm->TargetArmLength, TargetZoom_, DeltaTime, ZoomInterpSpeed_ );

	FVector clampedLocation = GetActorLocation();
	clampedLocation.X = FMath::Clamp( clampedLocation.X, MinMapBounds_.X, MaxMapBounds_.X );
	clampedLocation.Y = FMath::Clamp( clampedLocation.Y, MinMapBounds_.Y, MaxMapBounds_.Y );
	SetActorLocation( clampedLocation );

}

// Called to bind functionality to input
void AStrategyCamera::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent(playerInputComponent);

	playerInputComponent->BindAxis( "MoveForward", this, &AStrategyCamera::MoveForward );
	playerInputComponent->BindAxis( "MoveRight", this, &AStrategyCamera::MoveRight );

	playerInputComponent->BindAction( "ZoomIn", IE_Pressed, this, &AStrategyCamera::ZoomIn );
	playerInputComponent->BindAction( "ZoomOut", IE_Pressed, this, &AStrategyCamera::ZoomOut );

}

void AStrategyCamera::MoveForward( float value )
{
	if ( value != 0.0f )
	{
		AddMovementInput( FVector::ForwardVector, value );
	}
}
void AStrategyCamera::MoveRight( float value )
{
	if ( value != 0.0f )
	{
		AddMovementInput( FVector::RightVector, value );
	}
}

void AStrategyCamera::ZoomIn()
{
	TargetZoom_ = FMath::Clamp( TargetZoom_ - ZoomSpeed_, MinZoom_, MaxZoom_ );
}
void AStrategyCamera::ZoomOut()
{
	TargetZoom_ = FMath::Clamp( TargetZoom_ + ZoomSpeed_, MinZoom_, MaxZoom_ );
}