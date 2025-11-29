// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "UnitAIController.generated.h"


/** (Gregory-hub)
 * AIController for controlling a single unit */
UCLASS(Blueprintable)
class LORDS_FRONTIERS_API AUnitAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* pawn) override;
};
