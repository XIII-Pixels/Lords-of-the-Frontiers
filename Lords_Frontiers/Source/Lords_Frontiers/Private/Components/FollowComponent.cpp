// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FollowComponent.h"

#include "Units/Unit.h"

#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"

UFollowComponent::UFollowComponent()
{
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
		SnapToGround();
	}
}

void UFollowComponent::BeginPlay()
{
	Super::BeginPlay();

	Unit_ = Cast<AUnit>( GetOwner() );

	if ( PawnOwner )
	{
		CapsuleComponent_ = PawnOwner->FindComponentByClass<UCapsuleComponent>();
	}
}

void UFollowComponent::StartFollowing()
{
	if ( Unit_ && Unit_->FollowedTarget().IsValid() )
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
	if ( !Unit_ || !Unit_->FollowedTarget().IsValid() )
	{
		return;
	}

	const FVector targetLocation = Unit_->FollowedTarget()->GetActorLocation();
	const FVector actorLocation = GetActorLocation();
	FVector direction = ( targetLocation - actorLocation ).GetSafeNormal();
	direction.Z = 0.0f;

	CurrentDeviationYawSpeed_ += StreamRandom_.FRandRange( -1.0f, 1.0f ) * DeviationMaxRate_ * deltaTime;
	CurrentDeviationYaw_ += CurrentDeviationYawSpeed_ * deltaTime;
	CurrentDeviationYaw_ = FMath::Clamp( CurrentDeviationYaw_, -MaxDeviationAngle_, MaxDeviationAngle_ );
	const FRotator rot( 0, CurrentDeviationYaw_, 0 );
	direction = rot.RotateVector( direction );

	AddInputVector( direction );
}

void UFollowComponent::SnapToGround()
{
	if ( !PawnOwner || !CapsuleComponent_ )
	{
		return;
	}

	float halfHeight = CapsuleComponent_->GetScaledCapsuleHalfHeight();
	FVector location = PawnOwner->GetActorLocation();
	location.Z -= halfHeight;

	FVector start = location + FVector( 0, 0, SnapToGroundDistance_ );
	FVector end = location - FVector( 0, 0, SnapToGroundDistance_ );

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( PawnOwner );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Visibility, params ) )
	{
		FVector groundLocation = hit.ImpactPoint;
		location.Z = groundLocation.Z + halfHeight;
		PawnOwner->SetActorLocation( location );
	}
}

void UFollowComponent::Sway( float deltaTime )
{
	if ( IsValid( Unit_ ) && Unit_.Get()->VisualMesh() )
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

void UFollowComponent::RotateForward( float deltaTime )
{
	FRotator targetRotation = Velocity.Rotation();
	FRotator currentRotation = PawnOwner->GetActorRotation();

	FRotator newRotation = FMath::RInterpConstantTo( currentRotation, targetRotation, deltaTime, RotationSpeed_ );
	newRotation.Pitch = 0.0f;

	PawnOwner->SetActorRotation( newRotation );
}
