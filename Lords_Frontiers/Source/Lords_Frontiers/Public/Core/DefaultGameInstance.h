// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/LevelSubsystem/LevelsDataAsset.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "DefaultGameInstance.generated.h"

class ACardFeedbackPopup;

/** (Gregory-hub) */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UDefaultGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	TSubclassOf<ACardFeedbackPopup> GetCardFeedbackPopupClass() const
	{
		return CardFeedbackPopupClass_;
	}

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TObjectPtr<ULevelsDataAsset> Levels_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Cards" )
	TSubclassOf<ACardFeedbackPopup> CardFeedbackPopupClass_;
};
