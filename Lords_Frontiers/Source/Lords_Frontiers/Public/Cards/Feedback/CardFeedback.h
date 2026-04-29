#pragma once

#include "Cards/Visuals/CardVisualTypes.h"

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CardFeedback.generated.h"

class AActor;
class UTexture2D;

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

	UFUNCTION( BlueprintCallable, Category = "Card|Feedback",
		meta = ( WorldContext = "worldContextObject" ) )
	static void PlayVisuals(
		const UObject* worldContextObject,
		const FCardVisualConfig& config,
		AActor* owner,
		AActor* target );

	UFUNCTION( BlueprintCallable, Category = "Card|Feedback",
		meta = ( WorldContext = "worldContextObject" ) )
	static FCardVisualHandle BeginStickyVisual(
		const UObject* worldContextObject,
		const FCardVisualConfig& config,
		AActor* owner,
		AActor* target );

	UFUNCTION( BlueprintCallable, Category = "Card|Feedback",
		meta = ( WorldContext = "worldContextObject" ) )
	static void StopStickyVisual( const UObject* worldContextObject, FCardVisualHandle handle );
};
