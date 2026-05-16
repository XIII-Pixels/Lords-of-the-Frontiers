#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "LeaderboardConfig.generated.h"

USTRUCT( BlueprintType, meta = ( DisplayName = "Пресет-запись лидерборда" ) )
struct LORDS_FRONTIERS_API FLeaderboardSeedEntry
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Имя" ) )
	FString PlayerName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( DisplayName = "Очки" ) )
	int64 Score = 0;
};

UCLASS( BlueprintType, meta = ( DisplayName = "Конфиг таблицы лидеров" ) )
class LORDS_FRONTIERS_API ULeaderboardConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Лидерборд", meta = ( DisplayName = "Записи (Имя + Очки)", TitleProperty = "PlayerName" ) )
	TArray<FLeaderboardSeedEntry> Entries;
};
