#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CardFeedback.generated.h"

class ACardFeedbackPopup;

UCLASS()
class LORDS_FRONTIERS_API UCardFeedback : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Card|Feedback",
		meta = ( WorldContext = "worldContextObject" ) )
	static void ShowIconOnActor(
		const UObject* worldContextObject,
		AActor* owner,
		UTexture2D* icon,
		float durationSeconds = 1.0f,
		float floatHeight = 120.f );

private:
	static TSubclassOf<ACardFeedbackPopup> ResolvePopupClass( const UObject* worldContextObject );
};
