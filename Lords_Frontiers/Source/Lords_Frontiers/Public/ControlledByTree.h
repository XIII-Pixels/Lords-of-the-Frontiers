// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ControlledByTree.generated.h"

class UBehaviorTree;
enum class ETeam : uint8;

UINTERFACE( Blueprintable )
class LORDS_FRONTIERS_API UControlledByTree : public UInterface
{
	GENERATED_BODY()
};

/** (Gregory-hub)
 *  Interface for actors that are controlled by behavior tree */
class LORDS_FRONTIERS_API IControlledByTree
{
	GENERATED_BODY()

public:
	virtual TObjectPtr<UBehaviorTree> BehaviorTree() const = 0;
};
