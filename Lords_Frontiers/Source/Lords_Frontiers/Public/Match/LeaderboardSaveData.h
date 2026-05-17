#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "Lords_Frontiers/Public/Match/MatchStats.h"

#include "LeaderboardSaveData.generated.h"

UCLASS()
class LORDS_FRONTIERS_API ULeaderboardSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FLeaderboardEntry> Entries;
};
