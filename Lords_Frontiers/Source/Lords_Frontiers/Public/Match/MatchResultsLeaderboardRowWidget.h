#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "Lords_Frontiers/Public/Match/MatchStats.h"

#include "MatchResultsLeaderboardRowWidget.generated.h"

class UTextBlock;

UCLASS( BlueprintType, meta = ( DisplayName = "Строка таблицы лидеров" ) )
class LORDS_FRONTIERS_API UMatchResultsLeaderboardRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Строка" )
	void ApplyEntry( int32 rank, const FLeaderboardEntry& entry, FLinearColor color, const FText& formattedScore );

	UFUNCTION( BlueprintPure, Category = "Строка" )
	const FLeaderboardEntry& GetEntry() const
	{
		return CachedEntry;
	}

	UFUNCTION( BlueprintPure, Category = "Строка" )
	int32 GetRank() const
	{
		return CachedRank;
	}

	UFUNCTION( BlueprintPure, Category = "Строка" )
	bool IsPlayerRow() const
	{
		return CachedEntry.bIsCurrentPlayer;
	}

	UFUNCTION( BlueprintImplementableEvent, Category = "Строка", meta = ( DisplayName = "После заполнения строки" ) )
	void OnEntryApplied( int32 rank, const FLeaderboardEntry& entry, FLinearColor color );

protected:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> RankText = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> PlayerNameText = nullptr;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> ScoreText = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Строка" )
	FLeaderboardEntry CachedEntry;

	UPROPERTY( BlueprintReadOnly, Category = "Строка" )
	int32 CachedRank = 0;
};
