#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Lords_Frontiers/Public/Cards/CardTypes.h"
#include "Lords_Frontiers/Public/Match/LeaderboardConfig.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Lords_Frontiers/Public/Units/Unit.h"

#include "MatchScoringConfig.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Конфиг очков матча" ) )
class LORDS_FRONTIERS_API UMatchScoringConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Волны", meta = ( ClampMin = "0", DisplayName = "Очки за пройденную волну", ToolTip = "Базовое количество очков за каждую пройденную волну." ) )
	int32 PointsPerWave = 100;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Волны", meta = ( ClampMin = "0", DisplayName = "Доп. очки за номер волны", ToolTip = "Это значение умножается на номер волны и добавляется к награде. Чем дальше зашёл, тем дороже волна." ) )
	int32 PointsPerWaveScaling = 10;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Враги", meta = ( ClampMin = "0", DisplayName = "Очки за обычного врага (по умолчанию)", ToolTip = "Применяется ко всем классам врагов, для которых нет переопределения ниже." ) )
	int32 DefaultPointsPerEnemy = 5;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Враги", meta = ( ClampMin = "0", DisplayName = "Очки за босса (по умолчанию)", ToolTip = "Применяется к боссам без явного переопределения." ) )
	int32 DefaultPointsPerBoss = 500;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Враги", meta = ( DisplayName = "Очки за класс врага", ToolTip = "Переопределение базовых очков для конкретных классов врагов." ) )
	TMap<TSubclassOf<AUnit>, int32> PointsPerEnemyClass;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Урон", meta = ( ClampMin = "0.0", DisplayName = "Очков за единицу урона", ToolTip = "0 = не учитывать урон в счёте." ) )
	float PointsPerDamage = 0.05f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Башни", meta = ( DisplayName = "Очки за тип башни", ToolTip = "Бонус за каждую построенную башню по типу." ) )
	TMap<EDefensiveTowerType, int32> PointsPerTowerType;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Очки|Ресурсы", meta = ( DisplayName = "Очков за единицу ресурса", ToolTip = "Множитель очков за каждую заработанную единицу ресурса." ) )
	TMap<EResourceType, float> PointsPerResource;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лидерборд", meta = ( ClampMin = "1", DisplayName = "Размер таблицы лидеров" ) )
	int32 LeaderboardSize = 10;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лидерборд", meta = ( DisplayName = "Слот сохранения", ToolTip = "Имя слота, в котором USaveGame будет хранить таблицу лидеров." ) )
	FString LeaderboardSlotName = TEXT( "Leaderboard" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лидерборд", meta = ( DisplayName = "Конфиг записей", ToolTip = "DataAsset с пресет-записями (Имя + Очки), которые подмешиваются в таблицу лидеров." ) )
	TObjectPtr<ULeaderboardConfig> LeaderboardConfig = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лидерборд", meta = ( DisplayName = "Имя игрока в таблице", ToolTip = "Имя, под которым запись игрока попадает в таблицу лидеров." ) )
	FString PlayerEntryName = TEXT( "Ты" );
};
