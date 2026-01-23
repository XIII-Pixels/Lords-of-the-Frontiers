// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FollowComponent.h"

#include "Units/Unit.h"

#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"

UFollowComponent::UFollowComponent()
{
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
		MoveTowardsTarget();
	}

	if ( !Velocity.IsNearlyZero() )
	{
		RotateForward( deltaTime );
		SnapToNavMeshGround();
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
	if ( Unit_ && Unit_->FollowedTarget() )
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

void UFollowComponent::MoveTowardsTarget()
{
	if ( !Unit_ || !IsValid( Unit_->FollowedTarget() ) )
	{
		return;
	}

	FVector targetLocation = Unit_->FollowedTarget()->GetActorLocation();
	FVector actorLocation = GetActorLocation();
	FVector direction = ( targetLocation - actorLocation ).GetSafeNormal();
	direction.Z = 0.0f;

	AddInputVector( direction );
}

void UFollowComponent::SnapToNavMeshGround()
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

void UFollowComponent::RotateForward( float deltaTime )
{
	FRotator targetRotation = Velocity.Rotation();
	FRotator currentRotation = PawnOwner->GetActorRotation();

	FRotator newRotation = FMath::RInterpConstantTo( currentRotation, targetRotation, deltaTime, RotationSpeed_ );
	newRotation.Pitch = 0.0f;

	PawnOwner->SetActorRotation( newRotation );
}
