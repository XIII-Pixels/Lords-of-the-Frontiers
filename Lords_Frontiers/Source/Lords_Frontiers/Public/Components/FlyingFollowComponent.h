// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/FollowComponent.h"
#include "CoreMinimal.h"

#include "FlyingFollowComponent.generated.h"

UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UFlyingFollowComponent : public UFollowComponent
{
	GENERATED_BODY()

public:
	UFlyingFollowComponent();

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Flying", meta = ( ClampMin = "0", Units = "cm" ) )
	float FlightAltitude_ = 400.0f;

protected:
	virtual void BeginPlay() override;
	virtual void SnapToGround() const override;

	virtual FVector ProjectMoveLocation( const FVector& location ) const override
	{
		return FVector( location.X, location.Y, 0.0f );
	}
};