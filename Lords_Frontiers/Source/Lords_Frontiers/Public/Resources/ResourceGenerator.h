#pragma once

#include "GameResource.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "ResourceGenerator.generated.h"

class UResourceManager;

// Can be stored within GameState, PlayerController, or ResourceManager itself.
UCLASS( BlueprintType, Blueprintable )
class LORDS_FRONTIERS_API UResourceGenerator : public UObject
{
	GENERATED_BODY()

public:
	UResourceGenerator();

	void Initialize( UResourceManager* manager );

	void SetProductionConfig( const FResourceProduction& Config );

	TMap<EResourceType, int32> GetTotalProduction() const;

	void GenerateNow();

protected:
	UPROPERTY()
	TMap<EResourceType, int32> BaseProduction_;

	UPROPERTY()
	TMap<EResourceType, int32> BonusProduction_;

private:
	void ProcessGeneration();

	UPROPERTY()
	UResourceManager* ResourceManager_;
};
