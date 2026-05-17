#pragma once

#include "CoreMinimal.h"

#include "Lords_Frontiers/Public/Cards/CardTypes.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Lords_Frontiers/Public/Units/Unit.h"

#include "MatchStats.generated.h"

USTRUCT( BlueprintType, meta = ( DisplayName = "Статистика матча" ) )
struct LORDS_FRONTIERS_API FMatchStats
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Пройдено волн" ) )
	int32 WavesSurvived = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Убито врагов" ) )
	int32 EnemiesKilled = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Прожившие враги (дошли до базы)" ) )
	int32 EnemiesSurvived = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Убито боссов" ) )
	int32 BossesKilled = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Нанесено урона" ) )
	int64 DamageDealt = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Построено башен" ) )
	int32 TowersBuilt = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Убито врагов по классу" ) )
	TMap<TSubclassOf<AUnit>, int32> EnemiesKilledByClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Убито боссов по классу" ) )
	TMap<TSubclassOf<AUnit>, int32> BossesKilledByClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Построено башен по типу" ) )
	TMap<EDefensiveTowerType, int32> TowersBuiltByType;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Заработано ресурсов" ) )
	TMap<EResourceType, int64> ResourcesEarned;
};

USTRUCT( BlueprintType, meta = ( DisplayName = "Разбивка рейтинга" ) )
struct LORDS_FRONTIERS_API FScoreBreakdown
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Очки за волны" ) )
	int64 WavesScore = 0;

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Очки за обычных врагов" ) )
	int64 EnemiesScore = 0;

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Очки за боссов" ) )
	int64 BossesScore = 0;

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Очки за урон" ) )
	int64 DamageScore = 0;

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Очки за башни" ) )
	int64 TowersScore = 0;

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Очки за ресурсы" ))
	int64 ResourcesScore = 0;

	UPROPERTY( BlueprintReadOnly, meta = ( DisplayName = "Итого" ) )
	int64 Total = 0;
};

USTRUCT( BlueprintType, meta = ( DisplayName = "Запись таблицы лидеров" ) )
struct LORDS_FRONTIERS_API FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Имя игрока" ) )
	FString PlayerName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Общий счёт" ) )
	int64 Score = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Статистика" ) )
	FMatchStats Stats;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Дата записи" ) )
	FDateTime Timestamp;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Добавлено вручную" ) )
	bool bManual = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Запись текущего игрока" ) )
	bool bIsCurrentPlayer = false;

	bool operator==( const FLeaderboardEntry& other ) const
	{
		return Score == other.Score && Timestamp == other.Timestamp && PlayerName == other.PlayerName;
	}
};
