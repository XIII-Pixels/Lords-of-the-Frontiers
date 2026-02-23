#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EnemyAggroComponent.generated.h"

class ABuilding;
class USphereComponent;
class AUnit; // owner unit



/** (Artyom)
* Defines different enemy aggro profiles
* Each profile can have its own aggro radius, attack radius, priority classes, and filters.
* Used in EnemyUnit BP
* Contains a lot of debug logs, should be cleaned later
*/
UENUM( BlueprintType )
enum class EEnemyAggroProfile : uint8
{
	WolfGoblin UMETA( DisplayName = "Wolf-Goblin" ),
	OgroDog UMETA( DisplayName = "Ogro-Dog" ),
	ChiHuaGoblin UMETA( DisplayName = "ChiHua-Goblin" ),
	DuckDonDak UMETA( DisplayName = "Duck-Don-Dak" )
};

USTRUCT( BlueprintType )
struct FAggroProfileConfig
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	EEnemyAggroProfile Profile = EEnemyAggroProfile::WolfGoblin;

	// Aggro radius in cells. It will be multiplied by CellSizeCm
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro", meta = ( ClampMin = "0.0" ) )
	float AggroRadiusCells = 5.0f;

	// Priority list of building classes. Earlier entries = higher priority (index 0 = top priority).
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	TArray<TSubclassOf<ABuilding>> PriorityClasses;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	bool bIgnoreEconomyBuildings = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	bool bIgnoreTowers = false;
};

UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UEnemyAggroComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAggroComponent();

	// Which profile this component uses (choose in BP)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	EEnemyAggroProfile AggroProfile = EEnemyAggroProfile::WolfGoblin;

	// Size of one grid cell in cm.
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	float CellSizeCm = 100.0f;

	// Configs for profiles � GD fills one element per profile needed
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro" )
	TArray<FAggroProfileConfig> ProfileConfigs;

	// Draw debug spheres / found target 
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro|Debug" )
	bool bDebugDraw = false;

	// If true, component will create a USphereComponent
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Aggro|Debug" )
	bool bUseOverlapComponent = false;

	UPROPERTY( Transient )
	TObjectPtr<USphereComponent> OverlapSphereComponent = nullptr;

	// find best target and apply it to unit (AUnit::SetFollowedTarget).
	UFUNCTION( BlueprintCallable, Category = "Settings|Aggro" )
	void UpdateAggroTarget();

protected:
	virtual void BeginPlay() override;

	const FAggroProfileConfig* FindConfigForProfile( EEnemyAggroProfile profile ) const;

	int32 GetPriorityIndexForCandidate( const FAggroProfileConfig& config, ABuilding* candidate ) const;

	bool CandidatePassesFilters( const FAggroProfileConfig& config, ABuilding* candidate ) const;

	void UpdateOverlapSphereRadius();

	FORCEINLINE float CellsToCm( float Cells ) const
	{
		return Cells * CellSizeCm;
	}
};
