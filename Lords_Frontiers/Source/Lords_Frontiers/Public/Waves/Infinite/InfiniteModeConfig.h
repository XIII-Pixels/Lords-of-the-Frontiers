#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Lords_Frontiers/Public/Waves/EnemyBuff.h"
#include "Lords_Frontiers/Public/Waves/Infinite/EnemyPresetData.h"
#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteModeTypes.h"

#include "InfiniteModeConfig.generated.h"

class UCurveFloat;

USTRUCT( BlueprintType, meta = ( DisplayName = "Тема волны" ) )
struct FInfiniteTheme
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "ID темы", ToolTip = "Имя темы (для логов и UI). Например: Rush, Siege, AirRaid, EcoStrike, Splash, Elite, Mixed, Boss." ) )
	FName ThemeId = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "0.0", DisplayName = "Вес выпадения", ToolTip = "Вероятность выпадения этой темы относительно других в массиве." ) )
	float RollWeight = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "0", DisplayName = "Минимальная волна", ToolTip = "Тема не выпадает раньше этой волны. Используй чтобы скрыть Boss-тему до открытия первого босса." ) )
	int32 MinWaveIndex = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Только на спайках", ToolTip = "Тема выпадает только на 'жирных' волнах (каждая N-я). Удобно для Boss." ) )
	bool bSpikeOnly = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Множители тегов", ToolTip = "Список тегов с множителями веса. Например, Swarm × 4 = тема толпы." ) )
	TArray<FThemeTagWeight> TagWeights;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Доля бюджета на ядро (0..1)", ToolTip = "Какая часть бюджета волны идёт на 'ядро' — главный присет, задающий характер волны. 0.5 = половина бюджета." ) )
	float CoreBudgetFraction = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Шанс 'Spice' (0..1)", ToolTip = "Шанс добавить в конце волны один присет ВНЕ темы — для разнообразия." ) )
	float SpiceChance = 0.3f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Бафф темы", ToolTip = "Множители статов, добавляемые ко всем врагам волны этой темы. Стекаются с PresetBuff и масштабированием." ) )
	FEnemyBuff ThemeBuff;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "1", ClampMax = "8", DisplayName = "Мин. активных секторов", ToolTip = "Минимум секторов, из которых пойдут враги в этой теме." ) )
	int32 MinActiveSectors = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "1", ClampMax = "8", DisplayName = "Макс. активных секторов", ToolTip = "Максимум секторов, из которых пойдут враги в этой теме." ) )
	int32 MaxActiveSectors = 2;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Требовать противоположные секторы", ToolTip = "Если активных секторов >=2, второй должен быть на противоположной стороне кольца. Удобно для темы Siege." ) )
	bool bRequireOppositeSectors = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Требовать соседние секторы", ToolTip = "Если активных секторов >=2, второй должен быть соседом на кольце. Удобно для темы Splash." ) )
	bool bRequireAdjacentSectors = false;
};

USTRUCT( BlueprintType, meta = ( DisplayName = "Масштабирование статов" ) )
struct FInfiniteScalingConfig
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Базовый бафф (стартовая волна)", ToolTip = "Множители на самой первой бесконечной волне. Обычно все = 1.0." ) )
	FEnemyBuff BaseBuff;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Прирост за шаг", ToolTip = "Сколько добавляется к каждому множителю за один 'шаг' волны. Например, HP=0.08 значит '+8% HP за шаг'." ) )
	FEnemyBuff PerWaveDelta;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Потолок множителей", ToolTip = "Максимальные значения множителей. Без потолка скейлинг ломает баланс." ) )
	FEnemyBuff CapBuff;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "1", DisplayName = "Шаг (волн на одно усиление)", ToolTip = "1 = усиливать каждую волну. 5 = ступенька каждые 5 волн." ) )
	int32 StepEveryNWaves = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( DisplayName = "Кривая масштабирования", ToolTip = "Опционально: нелинейная кривая. X = номер шага, Y = коэффициент. Если не задана — линейный рост." ) )
	TObjectPtr<UCurveFloat> ScalingCurve = nullptr;
};

UCLASS( BlueprintType, meta = ( DisplayName = "Конфиг бесконечного режима" ) )
class LORDS_FRONTIERS_API UInfiniteModeConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Бюджет", meta = ( ClampMin = "0", DisplayName = "Стартовая волна", ToolTip = "Номер волны, с которой включается бесконечный режим. Волны до этого номера берутся из обычного WaveConfig." ) )
	int32 StartWaveIndex = 10;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Бюджет", meta = ( ClampMin = "0", DisplayName = "Стартовый бюджет", ToolTip = "Бюджет на самую первую бесконечную волну." ) )
	int32 StartingBudget = 100;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Бюджет", meta = ( ClampMin = "0", DisplayName = "Прирост бюджета за волну", ToolTip = "Сколько очков добавляется к бюджету каждую следующую волну (линейно)." ) )
	int32 BudgetPerWave = 25;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Бюджет", meta = ( DisplayName = "Кривая бюджета", ToolTip = "Опционально: кривая множителя сверху линейного прироста. X = номер бесконечной волны (от 0), Y = множитель." ) )
	TObjectPtr<UCurveFloat> BudgetCurve = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Спайк-волны", meta = ( ClampMin = "0", DisplayName = "Спайк каждые N волн", ToolTip = "Каждая N-я волна — 'жирная' с увеличенным бюджетом и доп. баффом. 0 = выключить спайки." ) )
	int32 SpikeWaveEvery = 5;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Спайк-волны", meta = ( ClampMin = "1.0", DisplayName = "Множитель бюджета спайка", ToolTip = "Во сколько раз увеличить бюджет на спайк-волне." ) )
	float SpikeBudgetMultiplier = 1.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Спайк-волны", meta = ( DisplayName = "Доп. бафф на спайке", ToolTip = "Дополнительные множители статов, применяемые поверх обычного скейлинга на спайк-волнах." ) )
	FEnemyBuff SpikeExtraBuff;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Контент", meta = ( DisplayName = "Присеты врагов", ToolTip = "Все доступные присеты, которые билдер может покупать." ) )
	TArray<TObjectPtr<UEnemyPresetData>> Presets;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Контент", meta = ( DisplayName = "Темы волн", ToolTip = "Все возможные темы. Для каждой волны бросается одна тема по их весам." ) )
	TArray<FInfiniteTheme> Themes;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Усиление врагов", meta = ( DisplayName = "Масштабирование статов", ToolTip = "Как именно враги усиливаются с каждой волной (HP, урон, скорость и т.д.)." ) )
	FInfiniteScalingConfig Scaling;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лимиты", meta = ( ClampMin = "1", DisplayName = "Макс. врагов на волну", ToolTip = "Жёсткий потолок числа врагов в волне (защита от лагов)." ) )
	int32 MaxEnemiesPerWave = 80;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лимиты", meta = ( ClampMin = "1", DisplayName = "Глобальный лимит покупок одного присета", ToolTip = "Сколько раз один и тот же присет можно купить в волне (если у самого присета MaxPerWave = 0)." ) )
	int32 GlobalMaxSamePresetPerWave = 4;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лимиты", meta = ( ClampMin = "0", DisplayName = "Память последних волн", ToolTip = "Сколько последних волн помнит билдер для антиповторов (по присетам и секторам)." ) )
	int32 RecentWaveMemory = 3;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лимиты", meta = ( ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Штраф за повтор (0..1)", ToolTip = "Множитель веса для присета/сектора, использованного в одной из последних волн. Меньше = сильнее давление антиповторов." ) )
	float RecencyPenalty = 0.4f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Секторы", meta = ( DisplayName = "Кольцо секторов (по часовой)", ToolTip = "Имена секторов в том порядке, в котором они идут по кругу карты (например: N, NE, E, SE, S, SW, W, NW). Используется для вычисления соседних и противоположных секторов." ) )
	TArray<FName> SectorRing;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Секторы", meta = ( DisplayName = "Разрешить летунам 'свободный' портал", ToolTip = "Один летающий присет за волну может вылететь из портала ВНЕ активных секторов — даёт лёгкий эффект сюрприза." ) )
	bool bAllowFlyerFreePortal = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Спецправила", meta = ( ClampMin = "0", DisplayName = "Гарантия 'Цель: Оборона' с волны", ToolTip = "Начиная с этой волны в каждой волне обязательно будет хотя бы один присет с тегом TargetsDefense / Ranged / Flying — иначе игрок мог бы просто уткнуть оборону в коридор." ) )
	int32 GuaranteeDefenseTargetingFromWave = 12;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Спецправила", meta = ( ClampMin = "0", DisplayName = "Макс. 'Цель: Экономика' за волну", ToolTip = "Сколько присетов с тегом TargetsEconomy разрешено в одной волне." ) )
	int32 MaxEcoHarassersPerWave = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Спецправила", meta = ( ClampMin = "0", DisplayName = "Перерыв между Apex (волн)", ToolTip = "Минимум сколько волн между двумя появлениями присетов с тегом Apex (сильнейший босс)." ) )
	int32 ApexCooldownWaves = 8;
};
