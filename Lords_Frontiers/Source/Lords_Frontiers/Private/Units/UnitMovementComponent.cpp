// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/UnitMovementComponent.h"

#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Units/Unit.h"

void UUnitMovementComponent::TickComponent(float deltaTime,
	ELevelTick tickType,
	FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	if ( !PawnOwner )
	{
		return;
	}

	// Only snap if the pawn is moving
	if ( !Velocity.IsNearlyZero() )
	{
		SnapToNavMeshGround();
	}
}

void UUnitMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( PawnOwner )
	{
		CapsuleComponent_ = PawnOwner->FindComponentByClass<UCapsuleComponent>();
	}
}

void UUnitMovementComponent::SnapToNavMeshGround()
{
	if ( !PawnOwner )
	{
		return;
	}

	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent( GetWorld() );
	if ( !navSys )
	{
		return;
	}

	FVector start = GetActorLocation() + FVector( 0, 0, SnapToGroundDistance );
	FVector end = GetActorLocation() - FVector( 0, 0, SnapToGroundDistance );

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( PawnOwner );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Visibility, params ) )
	{
		FVector groundLocation = hit.ImpactPoint;
		FVector newLocation = GetActorLocation();
		newLocation.Z = groundLocation.Z;
		PawnOwner->SetActorLocation( newLocation );
	}
}
