#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CardEffectHostComponent.generated.h"

class UCardDataAsset;
class UCardEffect;

USTRUCT()
struct FRegisteredCardEffect
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UCardDataAsset> SourceCard = nullptr;

	UPROPERTY()
	int32 EventIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UCardEffect> Effect = nullptr;
};

UCLASS( ClassGroup = ( Cards ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UCardEffectHostComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCardEffectHostComponent();

	void RegisterEffect( UCardDataAsset* card, int32 eventIndex, UCardEffect* effect );
	void UnregisterBySourceCard( UCardDataAsset* card );
	void ClearAll();

	const TArray<FRegisteredCardEffect>& GetActiveEffects() const
	{
		return Active_;
	}

	UFUNCTION( BlueprintCallable, Category = "Card|Counters" )
	int32 GetCounter( FName key ) const;

	UFUNCTION( BlueprintCallable, Category = "Card|Counters" )
	void SetCounter( FName key, int32 value );

	UFUNCTION( BlueprintCallable, Category = "Card|Counters" )
	int32 IncrementCounter( FName key );

	UFUNCTION( BlueprintPure, Category = "Card|Counters" )
	static FName MakeCounterKey( const UCardDataAsset* card, int32 eventIndex, FName localTag );

private:
	UPROPERTY()
	TArray<FRegisteredCardEffect> Active_;

	UPROPERTY()
	TMap<FName, int32> Counters_;
};
