#pragma once

#include "CoreMinimal.h"

#include "TutorialTypes.generated.h"

class UUserWidget;

UENUM( BlueprintType )
enum class ETutorialAdvance : uint8
{
	ClickAnywhere UMETA( DisplayName = "LMB anywhere" ),
	OnEndTurnPressed UMETA( DisplayName = "End-turn button pressed" ),
	OnWaveWon UMETA( DisplayName = "Wave won (Combat to Reward)" ),
	OnUpgradePicked UMETA( DisplayName = "Upgrade picked" ),
	None UMETA( DisplayName = "No advance (free play)" ),
};

USTRUCT( BlueprintType )
struct FTutorialStep
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial" )
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial" )
	ETutorialAdvance Advance = ETutorialAdvance::ClickAnywhere;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial" )
	TArray<FName> HighlightActorTags;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial",
	    meta = ( ToolTip = "Allow WASD/edge-scroll/rotation while this step is active. Zoom stays locked regardless." )
	)
	bool bAllowCameraMovement = false;
};
