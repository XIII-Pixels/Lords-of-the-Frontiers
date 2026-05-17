#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Lords_Frontiers/Public/Cards/CardTypes.h"
#include "Lords_Frontiers/Public/Match/MatchScoringConfig.h"
#include "Lords_Frontiers/Public/Match/MatchStats.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Lords_Frontiers/Public/Units/Unit.h"

#include "MatchStatsTracker.generated.h"

class ABuilding;
class ABuildManager;
class UGameLoopManager;
class UResourceManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnMatchScoreChanged, int64, totalScore, const FMatchStats&, stats );

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UMatchStatsTracker : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void BeginMatch( UMatchScoringConfig* config );

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void Reset();

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void OnWaveSurvived( int32 waveIndex );

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void OnEnemyKilled( TSubclassOf<AUnit> enemyClass, bool bIsBoss );

	UFUNCTION( BlueprintCallable, Category = "Статистика матча", meta = ( DisplayName = "Враг дошёл до базы (прожил)" ) )
	void OnEnemySurvived( TSubclassOf<AUnit> enemyClass );

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void OnDamageDealt( int32 amount );

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void OnTowerBuilt( EDefensiveTowerType towerType );

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	void OnResourceEarned( EResourceType resourceType, int32 amount );

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Статистика матча" )
	int64 ComputeScore() const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Статистика матча" )
	FScoreBreakdown ComputeBreakdown() const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Статистика матча" )
	int32 GetTowersBuiltOfType( EDefensiveTowerType towerType ) const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Статистика матча" )
	int64 GetResourceEarned( EResourceType resourceType ) const;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Статистика матча" )
	const FMatchStats& GetStats() const
	{
		return Stats;
	}

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Статистика матча" )
	UMatchScoringConfig* GetConfig() const
	{
		return Config;
	}

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	FLeaderboardEntry BuildEntry( const FString& playerName ) const;

	UFUNCTION( BlueprintCallable, Category = "Статистика матча" )
	bool FinalizeAndPush( const FString& playerName );

	UPROPERTY( BlueprintAssignable, Category = "Статистика матча" )
	FOnMatchScoreChanged OnScoreChanged;

	virtual void Deinitialize() override;

private:
	void Broadcast();

	void SubscribeToGameEvents();
	void UnsubscribeFromGameEvents();

	UFUNCTION()
	void HandleWaveChanged( int32 currentWave, int32 totalWaves );

	UFUNCTION()
	void HandleBuildingConfirmed( ABuilding* building, FIntPoint cell );

	UFUNCTION()
	void HandleResourceAdded( EResourceType type, int32 delta );

	void HandleUnitDied( AUnit* unit );
	void HandleDamageDealt( AActor* instigator, AActor* target, int damage, bool bIsSplash );

	UPROPERTY( Transient )
	TObjectPtr<UMatchScoringConfig> Config = nullptr;

	UPROPERTY( Transient )
	FMatchStats Stats;

	UPROPERTY( Transient )
	TWeakObjectPtr<UGameLoopManager> BoundGameLoop;

	UPROPERTY( Transient )
	TWeakObjectPtr<ABuildManager> BoundBuildManager;

	UPROPERTY( Transient )
	TWeakObjectPtr<UResourceManager> BoundResourceManager;

	FDelegateHandle UnitDiedHandle;
	FDelegateHandle DamageDealtHandle;
};
