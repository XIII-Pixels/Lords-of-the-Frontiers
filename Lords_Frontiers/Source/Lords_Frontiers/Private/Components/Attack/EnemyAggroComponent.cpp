#include "Lords_Frontiers/Public/Components/Attack/EnemyAggroComponent.h"

#include "DrawDebugHelpers.h"
#include "Lords_Frontiers/Public/Building/Building.h"
#include "Lords_Frontiers/Public/Building/DefensiveBuilding.h"
#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"

UEnemyAggroComponent::UEnemyAggroComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CellSizeCm = 100.f;
	AggroProfile = EEnemyAggroProfile::WolfGoblin;
	bDebugDraw = false;
	bUseOverlapComponent = false;
}

void UEnemyAggroComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( bUseOverlapComponent )
	{
		AActor* owner = GetOwner();
		if ( owner )
		{
			USphereComponent* sphere =
			    NewObject<USphereComponent>( owner, USphereComponent::StaticClass(), TEXT( "EnemyAggro_Sphere" ) );
			if ( sphere )
			{
				if ( owner->GetRootComponent() )
				{
					sphere->AttachToComponent(
					    owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform
					);
				}
				sphere->RegisterComponent();

				sphere->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
				sphere->SetCollisionObjectType( ECC_WorldDynamic );
				sphere->SetCollisionResponseToAllChannels( ECR_Ignore );
				sphere->SetCollisionResponseToChannel( ECC_Pawn, ECR_Overlap );
				sphere->SetGenerateOverlapEvents( true );

				OverlapSphereComponent = sphere;
				UpdateOverlapSphereRadius();

				UE_LOG(
				    LogTemp, Log, TEXT( "EnemyAggroComponent: Created overlap sphere component on '%s' (Radius=%f)" ),
				    *owner->GetName(), OverlapSphereComponent ? OverlapSphereComponent->GetUnscaledSphereRadius() : 0.f
				);
			}
			else
			{
				UE_LOG(
				    LogTemp, Warning, TEXT( "EnemyAggroComponent: Failed to create OverlapSphereComponent on '%s'" ),
				    *owner->GetName()
				);
			}
		}
	}
}

const FAggroProfileConfig* UEnemyAggroComponent::FindConfigForProfile( EEnemyAggroProfile profile ) const
{
	for ( const FAggroProfileConfig& contestant : ProfileConfigs )
	{
		if ( contestant.Profile == profile )
		{
			return &contestant;
		}
	}
	return nullptr;
}

bool UEnemyAggroComponent::CandidatePassesFilters( const FAggroProfileConfig& config, ABuilding* candidate ) const
{
	if ( !candidate )
	{
		if ( bDebugDraw )
		{
			UE_LOG( LogTemp, Log, TEXT( "CandidatePassesFilters: candidate is null" ) );
		}
		return false;
	}

	if ( bDebugDraw )
	{
		UE_LOG(
		    LogTemp, Log, TEXT( "CandidatePassesFilters: Candidate='%s' Class='%s'" ), *candidate->GetName(),
		    *candidate->GetClass()->GetName()
		);
	}

	if ( candidate->IsDestroyed() )
	{
		if ( bDebugDraw )
		{
			UE_LOG(
			    LogTemp, Log, TEXT( "CandidatePassesFilters: Rejecting '%s' because it is destroyed." ),
			    *candidate->GetName()
			);
		}
		return false;
	}

	// economy filter -> ResourceBuilding
	if ( config.bIgnoreEconomyBuildings )
	{
		const bool bIsResource = candidate->IsA( AResourceBuilding::StaticClass() );

		if ( bDebugDraw )
		{
			UE_LOG(
			    LogTemp, VeryVerbose, TEXT( "CandidatePassesFilters: bIgnoreEconomyBuildings=1, IsResource=%d" ),
			    bIsResource ? 1 : 0
			);
		}

		if ( bIsResource )
		{
			if ( bDebugDraw )
			{
				UE_LOG(
				    LogTemp, Log,
				    TEXT( "CandidatePassesFilters: Rejecting '%s' because it is a ResourceBuilding and config requests "
				          "ignoring economy." ),
				    *candidate->GetName()
				);
			}
			return false;
		}
	}

	// defensive filter -> DefensiveBuilding
	if ( config.bIgnoreTowers )
	{
		const bool bIsDefensive = candidate->IsA( ADefensiveBuilding::StaticClass() );
		if ( bDebugDraw )
		{
			UE_LOG(
			    LogTemp, Log, TEXT( "CandidatePassesFilters: bIgnoreTowers=1, IsDefensive=%d" ), bIsDefensive ? 1 : 0
			);
		}
		if ( bIsDefensive )
		{
			if ( bDebugDraw )
			{
				UE_LOG(
				    LogTemp, Log,
				    TEXT(
				        "CandidatePassesFilters: Rejecting '%s' because it is a DefensiveBuilding and config requests "
				        "ignoring towers."
				    ),
				    *candidate->GetName()
				);
			}
			return false;
		}
	}

	// passed all filters
	if ( bDebugDraw )
	{
		UE_LOG( LogTemp, Log, TEXT( "CandidatePassesFilters: Accepting '%s'" ), *candidate->GetName() );
	}
	return true;
}

int32 UEnemyAggroComponent::GetPriorityIndexForCandidate( const FAggroProfileConfig& config, ABuilding* candidate )
    const
{
	if ( !candidate )
	{
		return INT32_MAX;
	}
	if ( config.PriorityClasses.Num() == 0 )
	{
		return 0;
	}

	for ( int32 i = 0; i < config.PriorityClasses.Num(); ++i )
	{
		const TSubclassOf<ABuilding>& cls = config.PriorityClasses[i];
		if ( !cls )
		{
			continue;
		}
		if ( candidate->IsA( cls ) )
		{
			return i;
		}
	}
	return INT32_MAX;
}

void UEnemyAggroComponent::UpdateOverlapSphereRadius()
{
	if ( !OverlapSphereComponent )
	{
		return;
	}

	const FAggroProfileConfig* config = FindConfigForProfile( AggroProfile );
	if ( !config )
	{
		return;
	}

	const float radiusCm = CellsToCm( config->AggroRadiusCells );
	OverlapSphereComponent->SetSphereRadius( radiusCm, true );
}

void UEnemyAggroComponent::UpdateAggroTarget()
{
	if ( bDebugDraw )
	{
		UE_LOG( LogTemp, Log, TEXT( "Aggro tick from %s" ), *GetOwner()->GetName() );
	}

	AActor* owner = GetOwner();
	if ( !owner )
	{
		return;
	}

	AUnit* ownerUnit = Cast<AUnit>( owner );
	if ( !ownerUnit )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "EnemyAggroComponent: Owner is not AUnit, cannot set FollowedTarget (Owner=%s)" ),
		    owner ? *owner->GetName() : TEXT( "NULL" )
		);
		return;
	}

	const FAggroProfileConfig* config = FindConfigForProfile( AggroProfile );
	if ( !config )
	{
		UE_LOG( LogTemp, Warning, TEXT( "EnemyAggroComponent: Config invalid for profile %d" ), (int) AggroProfile );
		return;
	}
	if ( bDebugDraw )
	{
		UE_LOG(
		    LogTemp, Log, TEXT( "Aggro: Profile=%d AggroRadiusCells=%d IgnoreEconomy=%d IgnoreTowers=%d" ),
		    static_cast<int32>( config->Profile ), static_cast<int32>( config->AggroRadiusCells ),
		    config->bIgnoreEconomyBuildings ? 1 : 0, config->bIgnoreTowers ? 1 : 0
		);
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	const FVector myLoc = owner->GetActorLocation();
	float radiusCm = CellsToCm( config->AggroRadiusCells );

	const float MinRadius = 10.f;
	const float MaxRadius = 20000.f; // 20000 cm
	radiusCm = FMath::Clamp( radiusCm, MinRadius, MaxRadius );

	if ( bDebugDraw )
	{
		UE_LOG( LogTemp, Log, TEXT( "Aggro: Using radiusCm = %.1f (clamped)" ), radiusCm );
		DrawDebugSphere( world, myLoc, radiusCm, 24, FColor::Purple, false, 2.5f, 0, 4.0f );
	}

	// colliders settings
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	objectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Pawn ) );
	objectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_WorldStatic ) );
	objectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_WorldDynamic ) );

	objectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_GameTraceChannel1 ) );

	TArray<AActor*> ignoredActors;
	ignoredActors.Add( owner );

	TArray<AActor*> overlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
	    world, myLoc, radiusCm, objectTypes, ABuilding::StaticClass(), ignoredActors, overlappedActors
	);

	if ( bDebugDraw )
	{
		UE_LOG(
		    LogTemp, Log, TEXT( "Aggro: SphereOverlap found %d actors around '%s' (radius cm=%.1f)" ),
		    overlappedActors.Num(), *owner->GetName(), radiusCm
		);
	}

	int32 bestPriority = INT32_MAX;
	float bestDistSq = TNumericLimits<float>::Max();
	ABuilding* bestBuilding = nullptr;

	for ( AActor* actor : overlappedActors )
	{
		if ( !actor )
		{
			if ( bDebugDraw )
			{
				UE_LOG( LogTemp, Log, TEXT( "Aggro: overlapped actor null - skip" ) );
			}
			continue;
		}
		if ( bDebugDraw )
		{
			UE_LOG(
			    LogTemp, Log, TEXT( "Aggro: Candidate actor name='%s' class='%s'" ), *actor->GetName(),
			    actor->GetClass() ? *actor->GetClass()->GetName() : TEXT( "None" )
			);
		}

		ABuilding* building = Cast<ABuilding>( actor );
		if ( !building )
		{
			if ( bDebugDraw )
			{
				UE_LOG( LogTemp, Log, TEXT( "Aggro: Candidate '%s' is not ABuilding - skip" ), *actor->GetName() );
			}
			continue;
		}

		// log inheritance checks
		const bool bIsResource = building->IsA( AResourceBuilding::StaticClass() );
		const bool bIsDefensive = building->IsA( ADefensiveBuilding::StaticClass() );

		if ( bDebugDraw )
		{
			UE_LOG(
			    LogTemp, Log, TEXT( "Aggro: Candidate '%s' IsResource=%d IsDefensive=%d" ), *building->GetName(),
			    bIsResource ? 1 : 0, bIsDefensive ? 1 : 0
			);
		}

		// filter
		if ( !CandidatePassesFilters( *config, building ) )
		{
			if ( bDebugDraw )
			{
				UE_LOG(
				    LogTemp, Log, TEXT( "Aggro: Candidate '%s' filtered out by CandidatePassesFilters" ),
				    *building->GetName()
				);
			}
			continue;
		}

		const int32 pri = GetPriorityIndexForCandidate( *config, building );
		if ( pri == INT32_MAX )
		{
			if ( bDebugDraw )
			{
				UE_LOG(
				    LogTemp, Log, TEXT( "Aggro: Candidate '%s' has no matching priority class - skip" ),
				    *building->GetName()
				);
			}
			continue;
		}

		const float distSq = FVector::DistSquared( myLoc, building->GetActorLocation() );

		if ( bDebugDraw )
		{
			UE_LOG(
			    LogTemp, Log, TEXT( "Aggro: Candidate '%s' priority=%d distSq=%.1f" ), *building->GetName(), pri, distSq
			);
		}

		if ( pri < bestPriority || ( pri == bestPriority && distSq < bestDistSq ) )
		{
			bestPriority = pri;
			bestDistSq = distSq;
			bestBuilding = building;
			if ( bDebugDraw )
			{
				UE_LOG(
				    LogTemp, Log, TEXT( "Aggro: New best candidate '%s' (priority=%d distSq=%.1f)" ),
				    *building->GetName(), pri, distSq
				);
			}
		}
	}

	if ( bestBuilding != nullptr )
	{
		ownerUnit->SetFollowedTarget( bestBuilding );
	}
	else
	{
		ownerUnit->FollowPath();
	}

	if ( bDebugDraw )
	{
		if ( bestBuilding )
		{
			DrawDebugSphere( world, bestBuilding->GetActorLocation(), 50.f, 8, FColor::Green, false, 3.0f );
			UE_LOG(
			    LogTemp, Log, TEXT( "Aggro: Owner %s -> new target %s (priority=%d)" ), *owner->GetName(),
			    *bestBuilding->GetName(), bestPriority
			);
		}
		else
		{
			UE_LOG( LogTemp, Log, TEXT( "Aggro: Owner %s -> no target found" ), *owner->GetName() );
		}
	}
}
