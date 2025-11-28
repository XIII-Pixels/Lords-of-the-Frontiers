#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceTypes.h"
#include "ResourceGenerator.generated.h"

class UResourceManager;

// Actor that generates resources at a certain time interval
UCLASS()
class LORDS_FRONTIERS_API AResourceGenerator : public AActor
{
	GENERATED_BODY()

public:
	AResourceGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Starts the generation process
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void StartGeneration();

	// Stops the generation process
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void StopGeneration();

protected:
	// type of resource being generated
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	EResourceType ResourceType_;

	// Number of resources per tick
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "1"))
	int32 GenerationQuantity_;

	// Generation interval in seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.1"))
	float GenerationInterval_;

private:
	// basic logic of generation
	void Generate_();

	// Search for a resource manager 
	// for example, from PlayerController or GameState
	void FindResourceManager_();

private:
	FTimerHandle GenerationTimerHandle_;

	// Cached link to the resource manager. WeakPtr for security
	UPROPERTY()
	UResourceManager* ResourceManager_;

	// Default constant for the interval
	static constexpr float cDefaultInterval = 1.0f;
};