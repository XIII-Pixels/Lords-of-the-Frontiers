// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Building/Animation/ResourceAnimConfig.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "ResourceCollectionAnimComponent.generated.h"

/**
* Maxim
*/

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UResourceCollectionAnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UResourceCollectionAnimComponent();

	void StartAnimation( UResourceAnimConfig* animConfig, bool bIsRuined, float waveDelay );

	UFUNCTION( BlueprintPure, Category = "Settings|Animation" )
	bool IsAnimating() const
	{
		return bIsAnimating_;
	}
protected:
	virtual void
	TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

public:
	void FinishAnimation();
	void EvaluateInflatePhase( float totalAlpha, FVector& outScale ) const;
	void EvaluateDeflatePhase( float totalAlpha, FVector& outScale ) const;

	float GetCameraZoom() const;

	bool IsIconOnlyMode() const;

	UPROPERTY()
	TObjectPtr<UResourceAnimConfig> CachedAnimConfig_;

	FVector OriginalScale_;

	float Elapsed_ = 0.0f;
	float WaveDelay_ = 0.0f;
	bool bIsAnimating_ = false;
	bool bIsRuined_ = false;
};
