#pragma once

#include "CoreMinimal.h"

#include "GameResource.generated.h"

// Enum types of resources available in the game
UENUM( BlueprintType )
enum class EResourceType : uint8
{
	None = 0 UMETA( DisplayName = "None" ),
	Gold UMETA( DisplayName = "Gold" ),
	Food UMETA( DisplayName = "Food" ),
	Population UMETA( DisplayName = "Population" ),
	Progress UMETA( DisplayName = "Progress" ),
	Max UMETA( Hidden )
};

// A structure representing the storage unit of a resource (type + quantity)
USTRUCT( BlueprintType )
struct FGameResource
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Resource" )
	EResourceType Type;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Resource", meta = ( ClampMin = "0" ) )
	int32 Quantity;

	// Default c-tor
	FGameResource() : Type( EResourceType::None ), Quantity( 0 )
	{
	}

	// C-tor with params. Params in lowerCamelCase
	FGameResource( EResourceType type, int32 quantity ) : Type( type ), Quantity( quantity )
	{
	}

	// Comparison operator. Param 'other' in lowerCamelCase
	bool operator==( const FGameResource& other ) const
	{
		return Type == other.Type && Quantity == other.Quantity;
	}
};


USTRUCT( BlueprintType )
struct FResourceProduction
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Production" )
	int32 Gold = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Production" )
	int32 Food = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Production" )
	int32 Population = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Production" )
	int32 Progress = 0;

	TMap<EResourceType, int32> ToMap() const
	{
		TMap<EResourceType, int32> Map;
		Map.Add( EResourceType::Gold, Gold );
		Map.Add( EResourceType::Food, Food );
		Map.Add( EResourceType::Population, Population );
		Map.Add( EResourceType::Progress, Progress );
		return Map;
	}
};