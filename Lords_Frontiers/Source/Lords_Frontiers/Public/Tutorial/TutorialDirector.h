#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "TutorialDirector.generated.h"

class UTutorialConfig;

UCLASS()
class LORDS_FRONTIERS_API ATutorialDirector : public AActor
{
	GENERATED_BODY()

public:
	ATutorialDirector();

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Tutorial" )
	TObjectPtr<UTutorialConfig> Config;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Tutorial" )
	float StartDelay = 0.25f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type reason ) override;

private:
	void StartNow();

	FTimerHandle StartTimerHandle_;
};
