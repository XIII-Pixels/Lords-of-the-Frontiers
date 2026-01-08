#pragma once

#include "CoreMinimal.h"
#include "GameResource.generated.h"

// Enum types of resources available in the game
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Gold UMETA(DisplayName = "Gold"),
	Food UMETA(DisplayName = "Food"),
	Population UMETA(DisplayName = "Population"),
	Progress UMETA(DisplayName = "Progress"),
	MAX UMETA(Hidden)
};

// A structure representing the storage unit of a resource (type + quantity)
USTRUCT(BlueprintType)
struct FGameResource
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0"))
	int32 Quantity;

	// Default c-tor
	FGameResource()
		: Type(EResourceType::None)
		, Quantity(0)
	{
	}

	// C-tor with params. Params in lowerCamelCase
	FGameResource(EResourceType type, int32 quantity)
		: Type(type)
		, Quantity(quantity)
	{
	}

	// Comparison operator. Param 'other' in lowerCamelCase
	bool operator==(const FGameResource& other) const
	{
		return Type == other.Type && Quantity == other.Quantity;
	}
};