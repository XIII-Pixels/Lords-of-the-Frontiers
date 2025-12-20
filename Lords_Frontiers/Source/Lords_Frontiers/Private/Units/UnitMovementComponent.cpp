// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/UnitMovementComponent.h"

#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Units/Unit.h"

UUnitMovementComponent::UUnitMovementComponent()
{
}

void UUnitMovementComponent::TickComponent(float deltaTime,
	ELevelTick tickType,
	FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	if ( !PawnOwner )
	{
		return;
	}

	RotateForward( deltaTime );

	// Only snap if the pawn is moving
	if ( !Velocity.IsNearlyZero() )
	{
		SnapToNavMeshGround();
	}
}

void UUnitMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	MaxSpeed = MovementSpeed;

	if ( PawnOwner )
	{
		CapsuleComponent_ = PawnOwner->FindComponentByClass<UCapsuleComponent>();
	}
}

void UUnitMovementComponent::SnapToNavMeshGround()
{
	if ( !PawnOwner || !CapsuleComponent_ )
	{
		return;
	}

	float halfHeight = CapsuleComponent_->GetScaledCapsuleHalfHeight();
	FVector location = PawnOwner->GetActorLocation();
	location.Z -= halfHeight;

	FVector start = location + FVector( 0, 0, SnapToGroundDistance );
	FVector end = location - FVector( 0, 0, SnapToGroundDistance );

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

void UUnitMovementComponent::RotateForward(float deltaTime)
{
	if (Velocity.IsNearlyZero())
	{
		return;
	}
	FRotator targetRotation = Velocity.Rotation();
	FRotator currentRotation = PawnOwner->GetActorRotation();

	FRotator newRotation = FMath::RInterpConstantTo( currentRotation, targetRotation, deltaTime, RotationSpeed );

	PawnOwner->SetActorRotation( newRotation );
}
