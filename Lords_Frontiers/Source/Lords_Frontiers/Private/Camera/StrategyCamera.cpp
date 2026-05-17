#include "Camera/StrategyCamera.h"

#include "Core/DefaultGameInstance.h"
#include "Grid/GridManager.h"
#include "UI/GameHUD.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Sound/MusicAmbientManager.h"

AStrategyCamera::AStrategyCamera()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );

	SpringArm = CreateDefaultSubobject<USpringArmComponent>( TEXT( "SpringArm" ) );
	SpringArm->SetupAttachment( RootComponent );
	SpringArm->bDoCollisionTest = false;

	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>( TEXT( "Camera" ) );
	Camera->SetupAttachment( SpringArm );
	Camera->bUsePawnControlRotation = false;

	AudioListener = CreateDefaultSubobject<UCameraComponent>( TEXT( "AudioListener" ) );
	AudioListener->SetupAttachment( RootComponent );

	Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
	Camera->OrthoWidth = 2048.0f;
	SpringArm->TargetArmLength = 2000.0f;

	Camera->OrthoNearClipPlane = -2000.0f;
	Camera->OrthoFarClipPlane = 10000.0f;

	TargetZoom_ = 2048.0f;
	TargetYaw_ = CameraYaw_;
	CurrentYaw_ = CameraYaw_;

	if ( GetWorld() )
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(
		    GetWorld(), TEXT( "r.Shadow.Virtual.ResolutionLodBiasDirectional -2" )
		);
		UKismetSystemLibrary::ExecuteConsoleCommand( GetWorld(), TEXT( "r.Shadow.Virtual.ClipMargin 0.25" ) );
		UKismetSystemLibrary::ExecuteConsoleCommand(
		    GetWorld(), TEXT( "r.Shadow.Virtual.SMRT.RayCountDirectional 4" )
		);
	}
}

void AStrategyCamera::BeginPlay()
{
	Super::BeginPlay();

	Camera->SetProjectionMode( ProjectionMode_ );

	if ( ProjectionMode_ == ECameraProjectionMode::Orthographic )
	{
		Camera->OrthoWidth = InitialOrthoWidth_;
		TargetZoom_ = InitialOrthoWidth_;
		SpringArm->TargetArmLength = 3000.0f;
	}
	else
	{
		Camera->SetFieldOfView( FieldOfView_ );
		SpringArm->TargetArmLength = InitialTargetArmLength_;
		TargetZoom_ = InitialTargetArmLength_;
	}

	if ( APlayerController* pc = Cast<APlayerController>( GetController() ) )
	{
		if ( UEnhancedInputLocalPlayerSubsystem* subsystem =
		         ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( pc->GetLocalPlayer() ) )
		{
			if ( DefaultMappingContext )
			{
				subsystem->AddMappingContext( DefaultMappingContext, 0 );
			}
		}

		pc->SetViewTarget( this );
		pc->bShowMouseCursor = true;
		pc->SetInputMode( FInputModeGameAndUI() );
	}

	FVector startingLocation = GetActorLocation();
	startingLocation.Z = 0.0f;
	SetActorLocation( startingLocation );

	SpringArm->TargetArmLength = 3000.0f;
	SpringArm->SetRelativeRotation( FRotator( CameraPitch_, CameraYaw_, 0.0f ) );

	Camera->OrthoWidth = TargetZoom_;

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		this,
		&ThisClass::HandleAudioChangesOnZoom
	);

	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(
		handle,
		this,
		&ThisClass::HandleAudioChangesOnZoom,
		0.1f,
		false
	);

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
		}
	}

	MapCenter_ = ( MinMapBounds_ + MaxMapBounds_ ) * 0.5f;
	MaxMoveAreaExtents_ = ( MaxMapBounds_ - MinMapBounds_ ) * 0.5f;

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

void AStrategyCamera::SetZoomToMax()
{
	TargetZoom_ = MaxZoom_;
	if ( ProjectionMode_ == ECameraProjectionMode::Orthographic )
	{
		Camera->OrthoWidth = MaxZoom_;
	}
	else
	{
		SpringArm->TargetArmLength = MaxZoom_;
	}
}

// Called every frame
void AStrategyCamera::Tick( float deltaTime )
{
	Super::Tick( deltaTime );

	float realDeltaTime = FApp::GetDeltaTime();

	if ( ProjectionMode_ == ECameraProjectionMode::Orthographic )
	{
		Camera->OrthoWidth = FMath::FInterpTo( Camera->OrthoWidth, TargetZoom_, realDeltaTime, ZoomInterpSpeed_ );
	}
	else
	{
		SpringArm->TargetArmLength =
		    FMath::FInterpTo( SpringArm->TargetArmLength, TargetZoom_, realDeltaTime, ZoomInterpSpeed_ );
	}

	FRotator currentRot = SpringArm->GetRelativeRotation();
	FRotator targetRot = FRotator( CameraPitch_, TargetYaw_, 0.0f );
	FRotator newRot = FMath::RInterpTo( currentRot, targetRot, realDeltaTime, RotationSpeed_ * 0.1f );
	SpringArm->SetRelativeRotation( newRot );
	AudioListener->SetRelativeRotation( { 0.0f, newRot.Yaw, 0.0f } );

	if ( bEnableEdgeScrolling_ )
	{
		HandleEdgeScrolling();
	}

	FVector inputVector = ConsumeMovementInputVector().GetClampedToMaxSize( 1.0f );

	FVector deltaMove = inputVector * MoveSpeed_ * realDeltaTime;

	float currentZoom =
	    ( ProjectionMode_ == ECameraProjectionMode::Orthographic ) ? Camera->OrthoWidth : SpringArm->TargetArmLength;

	float zoomRange = FMath::Max( MaxZoom_ - MinZoom_, 1.0f );
	float zoomAlpha = FMath::Clamp( ( currentZoom - MinZoom_ ) / zoomRange, 0.0f, 1.0f );

	float curvedAlpha = FMath::Pow( zoomAlpha, BoundsCurveExponent_ );

	FVector2D currentExtents;
	if ( bIgnoreZoomBoundsCurve_ )
	{
		currentExtents = MaxMoveAreaExtents_;
	}
	else
	{
		currentExtents.X = FMath::Lerp( MaxMoveAreaExtents_.X, MinMoveAreaExtents_.X, curvedAlpha );
		currentExtents.Y = FMath::Lerp( MaxMoveAreaExtents_.Y, MinMoveAreaExtents_.Y, curvedAlpha );
	}

	FVector2D dynamicMinBounds = MapCenter_ - currentExtents;
	FVector2D dynamicMaxBounds = MapCenter_ + currentExtents;

	FVector currentLoc = GetActorLocation();

	currentLoc += deltaMove;

	FVector clampedLoc = currentLoc;
	clampedLoc.X = FMath::Clamp( clampedLoc.X, dynamicMinBounds.X, dynamicMaxBounds.X );
	clampedLoc.Y = FMath::Clamp( clampedLoc.Y, dynamicMinBounds.Y, dynamicMaxBounds.Y );

	if ( !GetActorLocation().Equals( clampedLoc, 0.1f ) )
	{
		SetActorLocation( clampedLoc );
	}
}

void AStrategyCamera::SetupPlayerInputComponent( UInputComponent* playerInputComponent )
{
	Super::SetupPlayerInputComponent( playerInputComponent );

	if ( UEnhancedInputComponent* EnhancedInputComponent =
	         CastChecked<UEnhancedInputComponent>( playerInputComponent ) )
	{
		EnhancedInputComponent->BindAction( MoveAction, ETriggerEvent::Triggered, this, &AStrategyCamera::Move );
		EnhancedInputComponent->BindAction( ZoomAction, ETriggerEvent::Triggered, this, &AStrategyCamera::Zoom );
		EnhancedInputComponent->BindAction( RotateAction, ETriggerEvent::Started, this, &AStrategyCamera::Rotate );

		if ( PauseAction )
		{
			EnhancedInputComponent->BindAction(
			    PauseAction, ETriggerEvent::Started, this, &AStrategyCamera::TogglePause
			);
		}
	}
}

void AStrategyCamera::Move( const FInputActionValue& value )
{
	if ( bIsCameraInputDisabled_ )
	{
		return;
	}

	FVector2D movementVector = value.Get<FVector2D>();

	const float cCameraYaw = SpringArm->GetComponentRotation().Yaw;
	const FRotator cYawRotation( 0, cCameraYaw, 0 );

	const FVector cForwardDir = FRotationMatrix( cYawRotation ).GetUnitAxis( EAxis::X );
	const FVector cRightDir = FRotationMatrix( cYawRotation ).GetUnitAxis( EAxis::Y );

	AddMovementInput( cForwardDir, movementVector.Y );
	AddMovementInput( cRightDir, movementVector.X );
}

void AStrategyCamera::Zoom( const FInputActionValue& value )
{
	if ( bIsCameraInputDisabled_ || bIsZoomDisabled_ )
	{
		return;
	}

	float zoomDirection = value.Get<float>();
	TargetZoom_ = FMath::Clamp( TargetZoom_ - ( zoomDirection * ZoomSpeed_ ), MinZoom_, MaxZoom_ );

	HandleAudioChangesOnZoom();
}

void AStrategyCamera::Rotate( const FInputActionValue& value )
{
	if ( bIsCameraInputDisabled_ )
	{
		return;
	}

	float direction = value.Get<float>();

	if ( direction != 0.0f )
	{
		TargetYaw_ += direction * 90.0f;
		TargetYaw_ = FRotator::NormalizeAxis( TargetYaw_ );
	}
}

void AStrategyCamera::HandleEdgeScrolling()
{
	if ( bIsCameraInputDisabled_ )
	{
		return;
	}

	if ( APlayerController* PC = Cast<APlayerController>( GetController() ) )
	{
		float mouseX, mouseY;
		if ( PC->GetMousePosition( mouseX, mouseY ) )
		{
			int32 viewportX, viewportY;
			PC->GetViewportSize( viewportX, viewportY );

			FVector2D movementInput( 0.f, 0.f );

			if ( mouseX <= EdgeScrollThreshold_ )
			{
				movementInput.X = -1.f;
			}
			else if ( mouseX >= viewportX - EdgeScrollThreshold_ )
			{
				movementInput.X = 1.f;
			}

			if ( mouseY <= EdgeScrollThreshold_ )
			{
				movementInput.Y = 1.f;
			}
			else if ( mouseY >= viewportY - EdgeScrollThreshold_ )
			{
				movementInput.Y = -1.f;
			}

			if ( !movementInput.IsZero() )
			{
				const float cCurrentYaw = SpringArm->GetComponentRotation().Yaw;
				const FRotator cYawRotation( 0, cCurrentYaw, 0 );
				const FVector cForwardDir = FRotationMatrix( cYawRotation ).GetUnitAxis( EAxis::X );
				const FVector cRightDir = FRotationMatrix( cYawRotation ).GetUnitAxis( EAxis::Y );

				AddMovementInput( cForwardDir, movementInput.Y );
				AddMovementInput( cRightDir, movementInput.X );
			}
		}
	}
}

void AStrategyCamera::TogglePause( const FInputActionValue& value )
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass( GetWorld(), FoundWidgets, UGameHUDWidget::StaticClass(), false );

	if ( FoundWidgets.Num() > 0 )
	{
		if ( UGameHUDWidget* HUD = Cast<UGameHUDWidget>( FoundWidgets[0] ) )
		{
			HUD->TogglePauseMenu();
		}
	}
}

void AStrategyCamera::HandleAudioChangesOnZoom()
{
	if ( const auto* gi = Cast<UDefaultGameInstance>( GetGameInstance() ) )
	{
		if ( auto* musicManager = gi->GetSubsystem<UMusicAmbientManager>() )
		{
			if ( TargetZoom_ >
			     MaxZoom_ - ( MaxZoom_ - MinZoom_ ) * WindyZoomPart_ ) // while in upper part of zoom range
			{
				musicManager->PlayWindAmbient();
			}
			else
			{
				musicManager->StopWindAmbient();
			}

			musicManager->AdjustAmbientVolume( 1.0f - ( TargetZoom_ - MinZoom_ ) / ( MaxZoom_ - MinZoom_ ) );
		}
	}
}
