#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyBuff.h"
#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteModeTypes.h"

#include "EnemyPresetData.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Присет врагов (бесконечный режим)" ) )
class LORDS_FRONTIERS_API UEnemyPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет", meta = ( ClampMin = "1", DisplayName = "Цена", ToolTip = "Сколько очков бюджета волны тратится на одну покупку этого присета." ) )
	int32 Cost = 10;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет", meta = ( DisplayName = "Класс врага", ToolTip = "Какой именно враг будет заспавнен." ) )
	TSubclassOf<AUnit> EnemyClass = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет", meta = ( ClampMin = "1", DisplayName = "Количество", ToolTip = "Сколько врагов появляется за одну покупку этого присета." ) )
	int32 Count = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Тайминг", meta = ( ClampMin = "0.0", DisplayName = "Задержка старта (сек)", ToolTip = "Через сколько секунд от начала волны появится первый враг этой покупки." ) )
	float StartDelay = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Тайминг", meta = ( ClampMin = "0.0", DisplayName = "Интервал спавна (сек)", ToolTip = "Пауза между появлением соседних врагов одной покупки. 0 = все мгновенно." ) )
	float SpawnInterval = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Доступность", meta = ( ClampMin = "0", DisplayName = "Минимальная волна", ToolTip = "С какой волны (по абсолютному номеру) этот присет может появиться. Используй для постепенного открытия новых врагов." ) )
	int32 MinWaveIndex = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Доступность", meta = ( ClampMin = "0.0", DisplayName = "Базовый вес", ToolTip = "Базовая вероятность выпадения этого присета в случайных бросках. Множители темы стекаются сверху." ) )
	float BaseWeight = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Теги", meta = ( Bitmask, BitmaskEnum = "/Script/Lords_Frontiers.EEnemyTag", DisplayName = "Теги", ToolTip = "Битовая маска тегов. Используется темами для подбора и спецправилами (TargetsDefense / Apex / Splash и т.п.)." ) )
	int32 TagMask = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Бафф", meta = ( DisplayName = "Личный бафф присета", ToolTip = "Множители статов, прибитые к самому присету (отдельно от глобального масштабирования волны и баффа темы — все три уровня перемножаются)." ) )
	FEnemyBuff PresetBuff;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Лимиты", meta = ( ClampMin = "0", DisplayName = "Макс. покупок за волну", ToolTip = "Сколько раз этот присет можно купить в одной волне. 0 = брать общий лимит из конфига." ) )
	int32 MaxPerWave = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Присет|Лимиты", meta = ( DisplayName = "Только как босс волны", ToolTip = "Если включено, билдер берёт этот присет ТОЛЬКО как ядро Boss-темы, никогда в filler. Используй для уникальных боссов (Утка, Ведьма)." ) )
	bool bBossOnly = false;

	FORCEINLINE bool HasTag( EEnemyTag tag ) const
	{
		return InfiniteMode::HasTag( TagMask, tag );
	}
};
