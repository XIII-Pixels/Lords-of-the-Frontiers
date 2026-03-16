// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FollowComponent.h"

#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Grid/GridManager.h"
#include "Units/Unit.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UFollowComponent::UFollowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SwayPhaseOffset_ = FMath::FRandRange( 0.0f, 2.0f * PI );
	StreamRandom_ = FRandomStream( FPlatformTime::Cycles64() );
}

void UFollowComponent::TickComponent(
    float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction
)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	if ( !PawnOwner )
	{
		return;
	}

	if ( bFollowTarget_ )
	{
		MoveTowardsTarget( deltaTime );
		Sway( deltaTime );
	}

	if ( !Velocity.IsNearlyZero() )
	{
		RotateForward( deltaTime );
	}

	ResolveUnitOnUnwalkableCell();
}

void UFollowComponent::BeginPlay()
{
	Super::BeginPlay();

	Unit_ = Cast<AUnit>( GetOwner() );

	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		Grid_ = core->GetGridManager();
	}

	if ( PawnOwner )
	{
		CapsuleComponent_ = PawnOwner->FindComponentByClass<UCapsuleComponent>();
	}

	SnapToGround();
}

void UFollowComponent::StartFollowing()
{
	if ( Unit_.IsValid() && Unit_->FollowedTarget().IsValid() )
	{
		bFollowTarget_ = true;
	}
}

void UFollowComponent::StopFollowing()
{
	bFollowTarget_ = false;
}

void UFollowComponent::SetMaxSpeed( float maxSpeed )
{
	MaxSpeed = maxSpeed;
}

void UFollowComponent::MoveTowardsTarget( float deltaTime )
{
	if ( !Unit_.IsValid() || !Unit_->FollowedTarget().IsValid() )
	{
		return;
	}

	const FVector targetLocation = Unit_->FollowedTarget()->GetActorLocation();
	const FVector actorLocation = GetActorLocation();
	CurrentDirection_ = ( targetLocation - actorLocation ).GetSafeNormal();
	CurrentDirection_.Z = 0.0f;

	CurrentDeviationYawSpeed_ += StreamRandom_.FRandRange( -1.0f, 1.0f ) * DeviationMaxRate_ * deltaTime;
	CurrentDeviationYaw_ += CurrentDeviationYawSpeed_ * deltaTime;
	CurrentDeviationYaw_ = FMath::Clamp( CurrentDeviationYaw_, -MaxDeviationAngle_, MaxDeviationAngle_ );
	const FRotator rot( 0, CurrentDeviationYaw_, 0 );
	CurrentDirection_ = rot.RotateVector( CurrentDirection_ );

	AddInputVector( CurrentDirection_ );
}

void UFollowComponent::SnapToGround() const
{
	if ( !PawnOwner )
	{
		return;
	}

	const AUnitAIManager* aiManager = nullptr;
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		aiManager = core->GetUnitAIManager();
		if ( !aiManager )
		{
			UE_LOG( LogTemp, Error, TEXT( "UFollowComponent::SnapToGround: UnitAIManager not found" ) );
			return;
		}
	}

	float halfHeight = 0.0f;
	if ( CapsuleComponent_.IsValid() )
	{
		halfHeight = CapsuleComponent_->GetScaledCapsuleHalfHeight();
	}

	const FVector groundLocation = { 0.0f, 0.0f, aiManager->GroundHeight() };

	FVector location = PawnOwner->GetActorLocation();
	location.Z = groundLocation.Z + halfHeight;
	PawnOwner->SetActorLocation( location );
}

void UFollowComponent::Sway( float deltaTime )
{
	if ( Unit_.IsValid() && Unit_.Get()->VisualMesh() )
	{
		float targetRoll = 0.0f;

		if ( Unit_.Get()->GetVelocity().Size() > 10.0f )
		{
			float time = GetWorld()->GetTimeSeconds();
			targetRoll = FMath::Sin( time * SwaySpeed_ + SwayPhaseOffset_ ) * SwayAmplitude_;
		}

		CurrentSwayRoll_ = FMath::FInterpTo( CurrentSwayRoll_, targetRoll, deltaTime, 10.0f );

		FRotator currentRot = Unit_.Get()->VisualMesh()->GetRelativeRotation();
		currentRot.Pitch = CurrentSwayRoll_;
		currentRot.Roll = 0.0f;
		Unit_.Get()->VisualMesh()->SetRelativeRotation( currentRot );
	}
}

void UFollowComponent::ResolveUnitOnUnwalkableCell()
{
	if ( !bAvoidUnwalkableCells_ || !Grid_.IsValid() || !Unit_.IsValid() || !CapsuleComponent_.IsValid() )
	{
		return;
	}

	const float cellSize = Grid_->GetCellSize();

	const FIntPoint currentCellCoords = Grid_->GetCellCoords( Unit_->GetActorLocation() );
	FVector currentCellCenter;
	Grid_->GetCellWorldCenter( currentCellCoords, currentCellCenter );

	TArray suspectedCoords {
	    currentCellCoords, currentCellCoords + FIntPoint( 1, 0 ), currentCellCoords + FIntPoint( -1, 0 ),
	    currentCellCoords + FIntPoint( 0, 1 ), currentCellCoords + FIntPoint( 0, -1 )
	};

	for ( const auto& cellCoords : suspectedCoords )
	{
		const FGridCell* cell = Grid_->GetCell( cellCoords.X, cellCoords.Y );
		if ( cell && !cell->bIsWalkable )
		{
			FVector cellCenter;
			Grid_->GetCellWorldCenter( cellCoords, cellCenter );

			const float dx = cellCenter.X - Unit_->GetActorLocation().X;
			const float dy = cellCenter.Y - Unit_->GetActorLocation().Y;

			const bool xIsInside = FMath::Abs( dx ) < cellSize / 2.0f;
			const bool yIsInside = FMath::Abs( dy ) < cellSize / 2.0f;

			if ( xIsInside && yIsInside && FMath::Abs( dx ) < FMath::Abs( dy ) )
			{
				const FVector location = Unit_->GetActorLocation();
				const float x = cellCenter.X - FMath::Sign( dx ) * cellSize / 2.0f;
				const FVector targetLocation = { x, location.Y, location.Z };
				Unit_->SetActorLocation( FMath::VInterpTo( location, targetLocation, GetWorld()->GetDeltaSeconds(), UnwalkablePushSpeed_ ) );
				return;
			}
			if ( xIsInside && yIsInside )
			{
				const FVector location = Unit_->GetActorLocation();
				const float y = cellCenter.Y - FMath::Sign( dy ) * cellSize / 2.0f;
				const FVector targetLocation = { location.X, y, location.Z };
				Unit_->SetActorLocation( FMath::VInterpTo( location, targetLocation, GetWorld()->GetDeltaSeconds(), UnwalkablePushSpeed_ ) );
				return;
			}
		}
	}
}

void UFollowComponent::RotateForward( float deltaTime )
{
	FRotator targetRotation = CurrentDirection_.Rotation();
	FRotator currentRotation = PawnOwner->GetActorRotation();

	FRotator newRotation = FMath::RInterpTo( currentRotation, targetRotation, deltaTime, RotationSpeed_ );
	newRotation.Pitch = 0.0f;

	PawnOwner->SetActorRotation( newRotation );
}
