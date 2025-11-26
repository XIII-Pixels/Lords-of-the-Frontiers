// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Attackable.h"

#include "GameFramework/Pawn.h"

#include "Unit.generated.h"


/** (Gregory-hub)
 * Base class for all units in a game
 * Can move and attack
 * Should be controlled by AI controller */
UCLASS(Abstract, Blueprintable)
class LORDS_FRONTIERS_API AUnit : public APawn, public IAttackable
{
	GENERATED_BODY()

public:
	AUnit();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** (Gregory-hub)
	 * Attack someone or something
	 * Calls IAttackable::TakeDamage on target
	 * @return true if an attack was performed, otherwise false */
	UFUNCTION(BlueprintCallable)
	virtual bool Attack(TScriptInterface<IAttackable> target);

	/** (Gregory-hub) */
	virtual bool TakeDamage(float damage) override;
};
