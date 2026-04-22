#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CardAoEField.generated.h"

class UStatusEffectDef;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ACardAoEField : public AActor
{
	GENERATED_BODY()

public:
	ACardAoEField();

	void Initialize(
		AActor* instigator,
		float radius,
		float duration,
		float tickInterval,
		int32 damagePerTick,
		UStatusEffectDef* statusPerTick );

protected:
	virtual void BeginPlay() override;
	virtual void Tick( float deltaTime ) override;

	void ApplyToEnemies();

	UPROPERTY( EditAnywhere, Category = "Field" )
	TObjectPtr<USceneComponent> SceneRoot_;

	float Radius_ = 0.f;
	float Duration_ = 0.f;
	float TickInterval_ = 1.f;
	int32 DamagePerTick_ = 0;

	UPROPERTY()
	TObjectPtr<UStatusEffectDef> StatusPerTick_;

	TWeakObjectPtr<AActor> Instigator_;

	float Elapsed_ = 0.f;
	float NextTickAt_ = 0.f;
};
