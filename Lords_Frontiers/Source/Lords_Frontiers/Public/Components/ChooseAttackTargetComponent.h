// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Building/Building.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "ChooseAttackTargetComponent.generated.h"

class ABuilding;

/** (Gregory-hub)
 * Finds closest building that unit should attack */
UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UChooseAttackTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	const TSet<TSubclassOf<ABuilding>>& TargetBuildingClasses() const
	{
		return TargetBuildingClasses_;
	}

protected:
	// Priority list of building classes
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings" )
	TSet<TSubclassOf<ABuilding>> TargetBuildingClasses_;
};
