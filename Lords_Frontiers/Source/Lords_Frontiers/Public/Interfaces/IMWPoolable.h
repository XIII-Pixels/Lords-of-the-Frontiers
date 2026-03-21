// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IMWPoolable.generated.h"

UINTERFACE(MinimalAPI)
class UMWPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LORDS_FRONTIERS_API IMWPoolable
{
	GENERATED_BODY()

public:

	virtual void ActivateFromPool() = 0;

	virtual void DeactivateToPool() = 0;
};
