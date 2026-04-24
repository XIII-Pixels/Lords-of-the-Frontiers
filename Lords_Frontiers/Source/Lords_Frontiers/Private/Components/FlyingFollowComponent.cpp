// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FlyingFollowComponent.h"

#include "Utilities/TraceChannelMappings.h"

#include "Components/CapsuleComponent.h"

UFlyingFollowComponent::UFlyingFollowComponent()
{
	bAvoidUnwalkableCells_ = false;
}

void UFlyingFollowComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( !PawnOwner )
	{
		return;
	}

	SnapToGround();
}

void UFlyingFollowComponent::SnapToGround() const
{
	if ( !PawnOwner )
	{
		return;
	}

	float halfHeight = 0.0f;
	if ( CapsuleComponent_.IsValid() )
	{
		halfHeight = CapsuleComponent_->GetScaledCapsuleHalfHeight();
	}

	FVector location = PawnOwner->GetActorLocation();
	location.Z = GroundHeight_ + halfHeight + FlightAltitude_;
	PawnOwner->SetActorLocation( location );
}