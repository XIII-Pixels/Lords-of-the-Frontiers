#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "MainBase.generated.h"

// The main building of the player. Loss of this building leads to defeat.
UCLASS()
class LORDS_FRONTIERS_API AMainBase : public ABuilding
{
	GENERATED_BODY()

public:
	AMainBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

private:
	// Example constant for base health
	static constexpr int32 cBaseMaxHealth = 1000;
};