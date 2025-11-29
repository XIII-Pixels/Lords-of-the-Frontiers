#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameResource.h"
#include "ResourceGenerator.generated.h"

class UResourceManager;

// Can be stored within GameState, PlayerController, or ResourceManager itself.
UCLASS(BlueprintType, Blueprintable)
class LORDS_FRONTIERS_API UResourceGenerator : public UObject
{
	GENERATED_BODY()

public:
	UResourceGenerator();

	// Explicit initialization to link with manager and world (Dependency Injection)
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize(UResourceManager* manager, UObject* worldContext);

	// Starts the generation timer (if needed)
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void StartGeneration();

	// Stops the generation timer
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void StopGeneration();

	// Manual generation call (e.g., for "After Wave" logic)
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateNow();

protected:
	// Type of resource being generated
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	EResourceType ResourceType_;

	// Number of resources per tick/action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "1"))
	int32 GenerationQuantity_;

	// Generation interval in seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.1"))
	float GenerationInterval_;

private:
	void ProcessGeneration();

	// Helper to safely get world from context
	UWorld* GetWorldContext() const;

private:
	FTimerHandle GenerationTimerHandle_;

	// ensuring GC handles references.
	UPROPERTY()
	UResourceManager* ResourceManager_;

	// Context to access TimerManager
	TWeakObjectPtr<UObject> WorldContext_;

	static constexpr float cDefaultInterval = 1.0f;
};