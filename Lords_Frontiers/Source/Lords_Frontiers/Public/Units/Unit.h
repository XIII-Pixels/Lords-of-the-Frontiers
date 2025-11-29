// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attackable.h"
#include "GameFramework/Pawn.h"
#include "Unit.generated.h"

class UBehaviorTree;


/** (Gregory-hub)
 * Base class for all units in a game
 * Can move and attack
 * Should be controlled by AI controller */
UCLASS(Abstract, Blueprintable)
class LORDS_FRONTIERS_API AUnit : public APawn, public IAttackable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void Attack(TScriptInterface<IAttackable> target);

	virtual void TakeDamage(float damage) override;

	const TObjectPtr<UBehaviorTree>& BehaviorTree() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI", meta = (DisplayPriority = 0))
	TObjectPtr<UBehaviorTree> BehaviorTree_;
};
