// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/AIModule/Classes/AIController.h"

#include "CoreMinimal.h"

#include "UnitAIController.generated.h"

/** (Gregory-hub)
 * AIController for controlling a single unit */
UCLASS( Blueprintable )
class LORDS_FRONTIERS_API AUnitAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void Tick( float DeltaSeconds ) override;

protected:
	virtual void OnPossess( APawn* pawn ) override;
};
