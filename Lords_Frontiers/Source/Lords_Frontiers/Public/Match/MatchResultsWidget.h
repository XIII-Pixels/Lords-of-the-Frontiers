#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "Core/GameSessionController.h"
#include "Lords_Frontiers/Public/Cards/CardTypes.h"
#include "Lords_Frontiers/Public/Match/MatchStats.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Sound/AudioEvent.h"
#include "Sound/AudioEventSource.h"
#include "UI/Widgets/MainMenuButtonWidget.h"

#include "MatchResultsWidget.generated.h"

class UMatchStatsTracker;
class ULeaderboardSubsystem;
class UMatchScoringConfig;
class UMatchResultsLeaderboardRowWidget;
class APlayerController;
class UTextBlock;
class UPanelWidget;

UCLASS( BlueprintType, meta = ( DisplayName = "Виджет результатов боя" ) )
class LORDS_FRONTIERS_API UMatchResultsWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Результаты" )
	void ShowResults( EGameResult result, const FString& playerName );

	UFUNCTION( BlueprintCallable, Category = "Результаты" )
	void Hide();

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	EGameResult GetSessionResult() const
	{
		return SessionResult;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	bool IsEndlessResult() const
	{
		return SessionResult == EGameResult::EndlessRun;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	const FMatchStats& GetMatchStats() const
	{
		return CachedStats;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	int64 GetTotalScore() const
	{
		return CachedBreakdown.Total;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	const FScoreBreakdown& GetScoreBreakdown() const
	{
		return CachedBreakdown;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Башни" )
	int32 GetTotalTowersBuilt() const
	{
		return CachedStats.TowersBuilt;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Башни" )
	int32 GetTowersBuiltOfType( EDefensiveTowerType towerType ) const
	{
		return CachedStats.TowersBuiltByType.FindRef( towerType );
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Ресурсы" )
	int64 GetResourceEarned( EResourceType resourceType ) const
	{
		return CachedStats.ResourcesEarned.FindRef( resourceType );
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Враги" )
	int32 GetEnemiesSurvived() const
	{
		return CachedStats.EnemiesSurvived;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Враги" )
	int32 GetRegularEnemiesKilled() const
	{
		return FMath::Max( 0, CachedStats.EnemiesKilled - CachedStats.BossesKilled );
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Враги" )
	int32 GetBossesKilled() const
	{
		return CachedStats.BossesKilled;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Враги" )
	int64 GetDamageDealt() const
	{
		return CachedStats.DamageDealt;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	int64 GetBestScore() const
	{
		return CachedBestScore;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	TArray<FLeaderboardEntry> GetTopEntries( int32 count ) const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд", meta = ( DisplayName = "Топ для виджета (пресеты + лучший/текущий)" ) )
	TArray<FLeaderboardEntry> GetLeaderboardView( int32 count ) const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд" )
	FLeaderboardEntry GetPlayerBestEntry() const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд" )
	FLeaderboardEntry GetPlayerCurrentEntry() const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд" )
	bool HasMultiplePlayerRuns() const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Формат", meta = ( DisplayName = "Формат числа (с запятыми)" ) )
	FText FormatNumber( int64 value ) const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Формат" )
	FText FormatCount( int32 value ) const
	{
		return FormatNumber( value );
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Формат" )
	FText FormatScore( int64 value ) const
	{
		return FormatNumber( value );
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд" )
	bool IsPlayerEntry( const FLeaderboardEntry& entry ) const
	{
		return entry.bIsCurrentPlayer;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд" )
	FLinearColor GetPlayerHighlightColor() const
	{
		return PlayerHighlightColor;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты|Лидерборд" )
	FLinearColor GetEntryColor( const FLeaderboardEntry& entry ) const
	{
		return entry.bIsCurrentPlayer ? PlayerHighlightColor : DefaultEntryColor;
	}

	UFUNCTION( BlueprintPure, Category = "Результаты" )
	int32 GetPlayerRank() const
	{
		return CachedPlayerRank;
	}

	UFUNCTION( BlueprintCallable, Category = "Результаты" )
	bool PushCurrentRunToLeaderboard( const FString& playerName );

	UFUNCTION( BlueprintCallable, Category = "Результаты" )
	bool AddManualEntry( const FString& playerName, int64 score );

	UFUNCTION( BlueprintImplementableEvent, Category = "Результаты" )
	void OnResultsReady( EGameResult result, int64 totalScore, const FMatchStats& stats );

	UFUNCTION( BlueprintImplementableEvent, Category = "Результаты" )
	void OnLeaderboardUpdated();

	UFUNCTION( BlueprintPure, Category = "Результаты|Состояние" )
	bool IsTrackerReady() const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Состояние" )
	bool IsLeaderboardReady() const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Состояние" )
	bool IsScoringConfigReady() const;

	UFUNCTION( BlueprintPure, Category = "Результаты|Состояние" )
	bool IsDataReady() const
	{
		return bDataReady;
	}

	UFUNCTION( BlueprintCallable, Category = "Результаты|Состояние" )
	bool ValidateState( bool bLogWarnings = true ) const;

	UFUNCTION( BlueprintCallable, Category = "Результаты|UI", meta = ( DisplayName = "Обновить надписи виджета" ) )
	void UpdateView();

	UFUNCTION( BlueprintCallable, Category = "Результаты|UI", meta = ( DisplayName = "Показать локальному игроку", WorldContext = "worldContextObject" ) )
	static UMatchResultsWidget* ShowForLocalPlayer( UObject* worldContextObject, TSubclassOf<UMatchResultsWidget> widgetClass, EGameResult result );

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void ApplyTextsToBoundWidgets();
	void RebuildLeaderboardRows();

	UMatchStatsTracker* GetTracker() const;
	ULeaderboardSubsystem* GetLeaderboard() const;

	void Refresh();

	UPROPERTY( BlueprintReadOnly, Category = "Результаты" )
	EGameResult SessionResult = EGameResult::EndlessRun;

	UPROPERTY( BlueprintReadOnly, Category = "Результаты" )
	FMatchStats CachedStats;

	UPROPERTY( BlueprintReadOnly, Category = "Результаты" )
	FScoreBreakdown CachedBreakdown;

	UPROPERTY( BlueprintReadOnly, Category = "Результаты" )
	int64 CachedBestScore = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Результаты" )
	int32 CachedPlayerRank = -1;

	UPROPERTY( BlueprintReadOnly, Category = "Результаты" )
	FString CachedPlayerName;

	UPROPERTY( BlueprintReadOnly, Category = "Результаты|Состояние" )
	bool bDataReady = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Результаты|Стиль", meta = ( DisplayName = "Цвет записи игрока" ) )
	FLinearColor PlayerHighlightColor = FLinearColor( 1.f, 0.78f, 0.25f, 1.f );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Результаты|Стиль", meta = ( DisplayName = "Цвет обычной записи" ) )
	FLinearColor DefaultEntryColor = FLinearColor::White;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Результаты|Лидерборд", meta = ( ClampMin = "1", DisplayName = "Сколько строк показывать в таблице" ) )
	int32 LeaderboardRowCount = 10;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Результаты|Лидерборд", meta = ( DisplayName = "Класс виджета строки (WBP_LeaderboardRow)" ) )
	TSubclassOf<UMatchResultsLeaderboardRowWidget> RowWidgetClass = nullptr;

	// ===== Auto-bound TextBlocks (имена в WBP_MatchResults должны совпадать) =====

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> ResultTitleText = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> WavesSurvivedValue = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> EnemiesKilledValue = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> BossesKilledValue = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> DamageDealtValue = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Speed_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Sniper_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Magic_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Mortar_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TowersTotalValue = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Resource_Mint_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Resource_Fish_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Resource_Population_Value = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TotalScoreValue = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> BestScoreValue = nullptr;

	// ===== Static labels: localized from ST_GameStrings (MatchResult.* keys) =====
	// Rename the plain "Text" blocks in WBP_MatchResults to these names and the
	// texts start following the active culture (see Doc/MatchResults_Localization.md).

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> BestScoreLabel = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> WavesSurvivedLabel = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> EnemiesKilledLabel = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> BossesKilledLabel = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> DamageDealtLabel = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Speed_Label = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Sniper_Label = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Magic_Label = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Tower_Mortar_Label = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TowersTotalLabel = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Resource_Mint_Label = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Resource_Fish_Label = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Resource_Population_Label = nullptr;

	// Section titles (replacements for the Text_Image_* pictures with baked-in text).

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Section_Enemies_Title = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Section_Towers_Title = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Section_Resources_Title = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Section_TotalScore_Title = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Section_Leaderboard_Title = nullptr;

	// Контейнер строк таблицы лидеров. Внутрь C++ кладёт по одной HorizontalBox-строке.
	// Подойдёт ScrollBox (рекомендую — будет скроллиться) или VerticalBox.
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UPanelWidget> LeaderboardContainer = nullptr;

	// Bottom buttons: WBP_MainMenuButton instances (same widget as the main menu),
	// named exactly RestartButton / MainMenuButton. C++ wires clicks, hover sounds
	// and assigns MatchResult.Restart / MatchResult.MainMenu label keys when the
	// instance has no LabelKey of its own. Plain UButton no longer binds here.
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UMainMenuButtonWidget> RestartButton = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UMainMenuButtonWidget> MainMenuButton = nullptr;

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

	FOnAudioEvent OnAudioEvent_;

	UFUNCTION()
	void HandleRestartClicked( EMainMenuButtonAction action );

	UFUNCTION()
	void HandleMainMenuClicked( EMainMenuButtonAction action );

	UFUNCTION()
	void HandleResultsButtonHovered( EMainMenuButtonAction action );
};
