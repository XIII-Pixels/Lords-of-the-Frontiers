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
	// Attack and damage
	UFUNCTION(BlueprintCallable)
	virtual void Attack(TScriptInterface<IAttackable> target);

	void TakeDamage(float damage) override;

	// Getters
	const TObjectPtr<UBehaviorTree>& BehaviorTree() const;

	const TObjectPtr<AActor>& Target() const;

protected:
	// Components
	UPROPERTY(EditDefaultsOnly, Category = "AI", meta = (DisplayPriority = 0))
	TObjectPtr<UBehaviorTree> BehaviorTree_;
	
	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayPriority = 0))
	TObjectPtr<AActor> Target_;
};
