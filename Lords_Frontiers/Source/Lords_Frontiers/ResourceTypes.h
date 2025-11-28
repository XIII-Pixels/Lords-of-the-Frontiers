#pragma once

#include "CoreMinimal.h"
#include "ResourceTypes.generated.h"

//Enum types of resources available in the game
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Gold UMETA(DisplayName = "Gold"),
	Wood UMETA(DisplayName = "Wood"),
	Mana UMETA(DisplayName = "Mana")
};

//A structure representing the storage unit of a resource (type + quantity)
USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0"))
	int32 Quantity;

	//Default c-tor
	FResource()
		: Type(EResourceType::None)
		, Quantity(0)
	{
	}

	//C-tor with params
	FResource(EResourceType InType, int32 InQuantity)
		: Type(InType)
		, Quantity(InQuantity)
	{
	}

	//Comparison operator for convenience of checks
	bool operator==(const FResource& Other) const
	{
		return Type == Other.Type && Quantity == Other.Quantity;
	}
};
