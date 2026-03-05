#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ISessionDataCollector.generated.h"

enum class EGameLoopPhase : uint8;

UINTERFACE( MinimalAPI, BlueprintType )
class USessionDataCollector : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for modular data collection in the session logger.
 * Implement this to add new tracking systems (challenges, achievements, live stats)
 * without modifying the core logger.
 *
 * Register via USessionLoggerSubsystem::RegisterCollector().
 */
class LORDS_FRONTIERS_API ISessionDataCollector
{
	GENERATED_BODY()

public:
	virtual void OnPhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
	{
	}

	virtual void OnTurnChanged( int32 Turn, int32 MaxTurns )
	{
	}

	virtual void OnWaveChanged( int32 Wave, int32 TotalWaves )
	{
	}

	virtual void OnGameEnded( bool bVictory )
	{
	}

	virtual void AppendToJson( TSharedPtr<FJsonObject> RootJson )
	{
	}

	virtual void ResetCollector()
	{
	}
};
