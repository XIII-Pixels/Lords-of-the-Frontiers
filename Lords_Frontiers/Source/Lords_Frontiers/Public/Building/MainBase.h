#pragma once

#include "ResourceBuilding.h"

#include "CoreMinimal.h"

#include "MainBase.generated.h"

// The main building of the player. Loss of this building leads to defeat.
UCLASS()
class LORDS_FRONTIERS_API AMainBase : public AResourceBuilding
{
	GENERATED_BODY()

public:
	AMainBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	virtual void OnDeath() override;

private:
	// Example constant for base health
	static constexpr int32 cBaseMaxHealth = 1000;

	UPROPERTY( Transient )
	bool bDefeatHandled = false;
};
