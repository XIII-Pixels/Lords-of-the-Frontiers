#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Lords_Frontiers/Public/Match/LeaderboardConfig.h"
#include "Lords_Frontiers/Public/Match/MatchStats.h"

#include "LeaderboardSubsystem.generated.h"

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API ULeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Лидерборд" )
	void Load( const FString& slotName );

	UFUNCTION( BlueprintCallable, Category = "Лидерборд" )
	void Save( const FString& slotName );

	UFUNCTION( BlueprintCallable, Category = "Лидерборд" )
	bool AddEntry( const FLeaderboardEntry& entry, int32 maxEntries );

	UFUNCTION( BlueprintCallable, Category = "Лидерборд" )
	bool AddManualEntry( const FString& playerName, int64 score, int32 maxEntries );

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	TArray<FLeaderboardEntry> GetTopN( int32 n ) const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	const TArray<FLeaderboardEntry>& GetAll() const
	{
		return Entries;
	}

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	int32 FindRankByScore( int64 score ) const;

	UFUNCTION( BlueprintCallable, Category = "Лидерборд" )
	bool RemoveAt( int32 index );

	UFUNCTION( BlueprintCallable, Category = "Лидерборд" )
	void Clear();

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	int64 GetBestScore() const;

	UFUNCTION( BlueprintCallable, Category = "Лидерборд", meta = ( DisplayName = "Загрузить пресет-записи из конфига" ) )
	void LoadSeedsFromConfig( const ULeaderboardConfig* config );

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	int32 GetSeedCount() const
	{
		return SeedEntries.Num();
	}

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	bool HasPlayerEntries() const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	FLeaderboardEntry GetPlayerBestEntry() const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд" )
	FLeaderboardEntry GetPlayerCurrentEntry() const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Лидерборд", meta = ( DisplayName = "Топ-N (пресеты + лучший/текущий игрока)" ) )
	TArray<FLeaderboardEntry> GetCombinedTopN( int32 n ) const;

private:
	void SortAndTrim( int32 maxEntries );

	UPROPERTY()
	TArray<FLeaderboardEntry> Entries;

	UPROPERTY()
	TArray<FLeaderboardEntry> SeedEntries;

	UPROPERTY()
	FString LastSlotName;
};
