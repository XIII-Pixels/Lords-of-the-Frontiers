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

	FGameResource() : Type( EResourceType::None ), Quantity( 0 )
	{
	}

	FGameResource( EResourceType type, int32 quantity ) : Type( type ), Quantity( quantity )
	{
	}

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

	/** Returns value for the given resource type. Returns 0 for None/Max. */
	int32 GetByType( EResourceType type ) const
	{
		switch ( type )
		{
		case EResourceType::Gold:
			return Gold;
		case EResourceType::Food:
			return Food;
		case EResourceType::Population:
			return Population;
		case EResourceType::Progress:
			return Progress;
		default:
			return 0;
		}
	}

	/** Modifies value for the given resource type by delta. Result clamped to >= 0. */
	void ModifyByType( EResourceType type, int32 delta )
	{
		switch ( type )
		{
		case EResourceType::Gold:
			Gold = FMath::Max( 0, Gold + delta );
			break;
		case EResourceType::Food:
			Food = FMath::Max( 0, Food + delta );
			break;
		case EResourceType::Population:
			Population = FMath::Max( 0, Population + delta );
			break;
		case EResourceType::Progress:
			Progress = FMath::Max( 0, Progress + delta );
			break;
		default:
			break;
		}
	}

	/**
	 * Like ModifyByType (result clamped to >= 0), but remembers in clampDebt the part that was
	 * cut off by the clamp. Applying -delta afterwards then restores exactly the previous value
	 * (e.g. maintenance 1, card -2 -> 0, revert +2 -> 1 instead of 2).
	 */
	void ModifyByTypeReversible( EResourceType type, int32 delta, FResourceProduction& clampDebt )
	{
		const int32 unclamped = GetByType( type ) - clampDebt.GetByType( type ) + delta;
		SetByType( type, FMath::Max( 0, unclamped ) );
		clampDebt.SetByType( type, FMath::Max( 0, -unclamped ) );
	}

	/** Sets value for the given resource type directly. */
	void SetByType( EResourceType type, int32 value )
	{
		switch ( type )
		{
		case EResourceType::Gold:
			Gold = value;
			break;
		case EResourceType::Food:
			Food = value;
			break;
		case EResourceType::Population:
			Population = value;
			break;
		case EResourceType::Progress:
			Progress = value;
			break;
		default:
			break;
		}
	}

	/** Returns true if all values are zero */
	bool IsZero() const
	{
		return Gold == 0 && Food == 0 && Population == 0 && Progress == 0;
	}

	TMap<EResourceType, int32> ToMap() const
	{
		TMap<EResourceType, int32> Map;
		Map.Add( EResourceType::Gold, Gold );
		Map.Add( EResourceType::Food, Food );
		Map.Add( EResourceType::Population, Population );
		Map.Add( EResourceType::Progress, Progress );
		return Map;
	}

	FResourceProduction operator+( const FResourceProduction& other ) const
	{
		FResourceProduction result;
		result.Gold = Gold + other.Gold;
		result.Food = Food + other.Food;
		result.Population = Population + other.Population;
		result.Progress = Progress + other.Progress;
		return result;
	}

	FResourceProduction operator-( const FResourceProduction& other ) const
	{
		FResourceProduction result;
		result.Gold = Gold - other.Gold;
		result.Food = Food - other.Food;
		result.Population = Population - other.Population;
		result.Progress = Progress - other.Progress;
		return result;
	}

	bool operator==( const FResourceProduction& other ) const
	{
		return Gold == other.Gold && Food == other.Food && Population == other.Population && Progress == other.Progress;
	}

	bool operator!=( const FResourceProduction& other ) const
	{
		return !( *this == other );
	}
};
