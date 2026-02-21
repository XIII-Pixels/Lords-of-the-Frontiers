#pragma once
#include "Lords_Frontiers/Public/Units/Unit.h"

#include "CoreMinimal.h"

#include "EnemyBuff.generated.h"
/*
(Maxim)
 */
USTRUCT( BlueprintType )
struct FEnemyBuff
{
	GENERATED_BODY()
	UPROPERTY( EditAnywhere, Category = "Settings|Buff" )
	int32 SpawnCount = 0;

	UPROPERTY( EditAnywhere, Category = "Settings|Buff|Multiply" )
	float HealthMultiplier = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Setting|Buff|Multiply" )
	float AttackRangeMultiplier = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Setting|Buff|Multiply" )
	float AttackDamageMultiplier = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Setting|Buff|Multiply" )
	float AttackCooldownMultiplier = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Setting|Buff|Multiply" )
	float MaxSpeedMultiplier = 1.0f;
};