// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/StrategyCamera.h"
#include "Grid/GridManager.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#include "GameFramework/FloatingPawnMovement.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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

	Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
	Camera->OrthoWidth = 2048.0f;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>( TEXT( "MovementComponent" ) );

	TargetZoom_ = 2048.0f;
}

// Called when the game starts or when spawned
void AStrategyCamera::BeginPlay()
{
	Super::BeginPlay();

	if ( APlayerController* pc = Cast<APlayerController>( GetController() ) )
	{
		if ( UEnhancedInputLocalPlayerSubsystem* Subsystem =
		         ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( pc->GetLocalPlayer() ) )
		{
			if ( DefaultMappingContext )
			{
				Subsystem->AddMappingContext( DefaultMappingContext, 0 );
			}
		}

		pc->SetViewTarget( this );
		pc->bShowMouseCursor = true;
		pc->SetInputMode( FInputModeGameAndUI() );
	}

	FVector startingLocation = GetActorLocation();
	startingLocation.Z = 0.0f;
	SetActorLocation( startingLocation );

	MovementComponent->MaxSpeed = MoveSpeed_;
	MovementComponent->Acceleration = MoveAcceleration_;
	MovementComponent->Deceleration = MoveDeceleration_;

	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->CameraLagSpeed = CameraLagSpeed_;
	SpringArm->SetRelativeRotation( FRotator( CameraPitch_, CameraYaw_, 0.0f ) );

	Camera->OrthoWidth = TargetZoom_;

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

	Camera->OrthoWidth = FMath::FInterpTo( Camera->OrthoWidth, TargetZoom_, DeltaTime, ZoomInterpSpeed_ );

	FVector currentLoc = GetActorLocation();
	FVector clampedLoc = currentLoc;

	clampedLoc.X = FMath::Clamp( clampedLoc.X, MinMapBounds_.X, MaxMapBounds_.X );
	clampedLoc.Y = FMath::Clamp( clampedLoc.Y, MinMapBounds_.Y, MaxMapBounds_.Y );

	if ( !currentLoc.Equals( clampedLoc, 0.1f ) )
	{
		SetActorLocation( clampedLoc );
	}
}

// Called to bind functionality to input
void AStrategyCamera::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent( playerInputComponent );

	if ( UEnhancedInputComponent* EnhancedInputComponent =
	         CastChecked<UEnhancedInputComponent>( playerInputComponent ) )
	{
		EnhancedInputComponent->BindAction( MoveAction, ETriggerEvent::Triggered, this, &AStrategyCamera::Move );
		EnhancedInputComponent->BindAction( ZoomAction, ETriggerEvent::Triggered, this, &AStrategyCamera::Zoom );
	}
}

void AStrategyCamera::Move( const FInputActionValue& Value )
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const float CameraYaw = SpringArm->GetComponentRotation().Yaw;
	const FRotator YawRotation( 0, CameraYaw, 0 );

	const FVector ForwardDir = FRotationMatrix( YawRotation ).GetUnitAxis( EAxis::X );
	const FVector RightDir = FRotationMatrix( YawRotation ).GetUnitAxis( EAxis::Y );

	AddMovementInput( ForwardDir, MovementVector.Y );
	AddMovementInput( RightDir, MovementVector.X );
}

void AStrategyCamera::Zoom( const FInputActionValue& Value )
{
	float zoomDirection = Value.Get<float>();

	TargetZoom_ = FMath::Clamp( TargetZoom_ - ( zoomDirection * ZoomSpeed_ ), MinZoom_, MaxZoom_ );
}
