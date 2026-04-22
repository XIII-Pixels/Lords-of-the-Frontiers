#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "StatusEffectTracker.generated.h"

class UStatusEffectDef;

USTRUCT()
struct FActiveStatus
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UStatusEffectDef> Def = nullptr;

	float ExpiresAt = 0.f;
	float NextTickAt = 0.f;
	float CachedOriginal = 0.f;
	float StackAmount = 0.f;
};

UCLASS( ClassGroup = ( Cards ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UStatusEffectTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectTracker();

	virtual void TickComponent( float dt, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void ApplyStatus( UStatusEffectDef* def );

	UFUNCTION( BlueprintPure, Category = "Status" )
	bool HasStatus( const UStatusEffectDef* def ) const;

	UFUNCTION( BlueprintPure, Category = "Status" )
	bool HasStatusTag( FName tag ) const;

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void RemoveStatus( UStatusEffectDef* def );

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void ClearAll();

	static UStatusEffectTracker* EnsureOn( AActor* actor );

private:
	UPROPERTY()
	TMap<FName, FActiveStatus> Active_;
};
