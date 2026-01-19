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

	void SetProductionConfig( const TArray<FGameResource>& Config );

	void GenerateNow();

	const TArray<FGameResource>& GetProduction() const
	{
		return ProductionResources_;
	}

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings" )
	TArray<FGameResource> ProductionResources_;

private:
	void ProcessGeneration();

	UPROPERTY()
	UResourceManager* ResourceManager_;
};
