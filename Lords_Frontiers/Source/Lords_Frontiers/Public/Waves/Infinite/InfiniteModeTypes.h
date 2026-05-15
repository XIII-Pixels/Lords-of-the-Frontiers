#pragma once

#include "CoreMinimal.h"

#include "InfiniteModeTypes.generated.h"

/*
 * Tags used by the infinite mode builder to classify enemy presets and to
 * drive theme weights. Stored as bitmask in UEnemyPresetData::TagMask.
 */
UENUM( BlueprintType, meta = ( Bitflags, UseEnumValuesAsMaskValuesInEditor = "false" ) )
enum class EEnemyTag : uint8
{
	Melee           = 0  UMETA( DisplayName = "Ближник" ),
	Ranged          = 1  UMETA( DisplayName = "Дальник" ),
	Swarm           = 2  UMETA( DisplayName = "Толпа (дешёвый)" ),
	Tank            = 3  UMETA( DisplayName = "Танк" ),
	Splash          = 4  UMETA( DisplayName = "Сплеш-урон" ),
	Flying          = 5  UMETA( DisplayName = "Летающий" ),
	Elite           = 6  UMETA( DisplayName = "Элита (мини-босс)" ),
	Boss            = 7  UMETA( DisplayName = "Босс" ),
	Apex            = 8  UMETA( DisplayName = "Апекс (сильнейший босс)" ),
	TargetsBase     = 9  UMETA( DisplayName = "Цель: База" ),
	TargetsDefense  = 10 UMETA( DisplayName = "Цель: Оборона" ),
	TargetsEconomy  = 11 UMETA( DisplayName = "Цель: Экономика" )
};

namespace InfiniteMode
{
	FORCEINLINE int32 TagBit( EEnemyTag tag )
	{
		return 1 << static_cast<int32>( tag );
	}

	FORCEINLINE bool HasTag( int32 tagMask, EEnemyTag tag )
	{
		return ( tagMask & TagBit( tag ) ) != 0;
	}
}

/*
 * Multiplier applied to a preset's base weight when the preset has the given
 * tag. Used inside FInfiniteTheme.
 */
USTRUCT( BlueprintType, meta = ( DisplayName = "Вес тега темы" ) )
struct FThemeTagWeight
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Тег", ToolTip = "Какой тег врага получает множитель веса в этой теме." ) )
	EEnemyTag Tag = EEnemyTag::Swarm;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "0.0", DisplayName = "Множитель", ToolTip = "Во сколько раз увеличить (>1) или уменьшить (<1) вес присета с этим тегом." ) )
	float Multiplier = 1.0f;
};
