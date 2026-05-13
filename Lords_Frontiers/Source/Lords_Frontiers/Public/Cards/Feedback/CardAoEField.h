#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CardAoEField.generated.h"

class UStatusEffectDef;
enum class EGameLoopPhase : uint8;

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
		UStatusEffectDef* statusPerTick,
		bool bDebugDrawRadius = false,
		const FColor& debugColor = FColor::Yellow );

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;
	virtual void Tick( float deltaTime ) override;

	void ApplyToEnemies();
	void ApplyVisualScale();

	UFUNCTION()
	void HandleGameLoopPhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase );

	UPROPERTY( EditAnywhere, Category = "Field" )
	TObjectPtr<USceneComponent> SceneRoot_;

	/**
	 * Effective radius (cm) the BP visual was authored at. The actor's
	 * SceneRoot is uniformly scaled by Radius_ / VisualReferenceRadius so
	 * the visual matches gameplay radius. Set this per-BP to whatever
	 * radius the decal/mesh/Niagara was designed for.
	 */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Field|Visual",
		meta = ( ClampMin = "1.0" ) )
	float VisualReferenceRadius = 200.f;

	/** If false, gameplay radius does not affect the actor scale. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Field|Visual" )
	bool bScaleVisualToRadius = true;

	float Radius_ = 0.f;
	float Duration_ = 0.f;
	float TickInterval_ = 1.f;
	int32 DamagePerTick_ = 0;

	bool bDebugDrawRadius_ = false;
	FColor DebugColor_ = FColor::Yellow;

	UPROPERTY()
	TObjectPtr<UStatusEffectDef> StatusPerTick_;

	TWeakObjectPtr<AActor> Instigator_;

	float Elapsed_ = 0.f;
	float NextTickAt_ = 0.f;
};
