#pragma once

#include "Projectiles/BaseProjectile.h"

#include "CoreMinimal.h"

#include "ParabolicProjectile.generated.h"

/**
 * Maxim
 */
UCLASS()
class LORDS_FRONTIERS_API AParabolicProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:
	AParabolicProjectile();

	virtual void ActivateFromPool() override;
	virtual void DeactivateToPool() override;

	virtual void Tick( float deltaTime ) override;

protected:

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Parabolic" )
	float ArcHeight_ = 300.0f;
	float CurrentArcHeight_ = 0.0f;


private:
	FVector StartLocation_;
	FVector TargetLocation_;
	float FlightProgress_ = 0.0f;
	float FlightDuration_ = 0.0f;
};