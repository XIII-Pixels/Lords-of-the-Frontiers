// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/LevelSubsystem/LevelsDataAsset.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "DefaultGameInstance.generated.h"

class UMusicDataAsset;
class UAmbientDataAsset;
class USoundDataAsset;
class UGameSaverConfig;
class ACardFeedbackPopup;
class ACardIconStrip;

/** (Gregory-hub) */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UDefaultGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TObjectPtr<ULevelsDataAsset> Levels;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Saving" )
	TObjectPtr<UGameSaverConfig> GameSaverConfig;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Sound" )
	TObjectPtr<UMusicDataAsset> Music;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Sound" )
	TObjectPtr<UAmbientDataAsset> Ambient;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Sound" )
	TObjectPtr<USoundDataAsset> SoundData;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Sound" )
	TObjectPtr<USoundAttenuation> DefaultAttenuation;

	TSubclassOf<ACardFeedbackPopup> GetCardFeedbackPopupClass() const
	{
		return CardFeedbackPopupClass_;
	}

	TSubclassOf<ACardIconStrip> GetCardIconStripClass() const
	{
		return CardIconStripClass_;
	}

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Cards" )
	TSubclassOf<ACardFeedbackPopup> CardFeedbackPopupClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Cards" )
	TSubclassOf<ACardIconStrip> CardIconStripClass_;
};
