#include "Lords_Frontiers/Public/Components/Attack/EnemyAggroComponent.h"
#include "Lords_Frontiers/Public/Building/Building.h"
#include "Lords_Frontiers/Public/Building/DefensiveBuilding.h"
#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
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
			USphereComponent* sphere = NewObject<USphereComponent>( owner, USphereComponent::StaticClass(), TEXT( "EnemyAggro_Sphere" ) );
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
			}
		}
	}
}

const FAggroProfileConfig* UEnemyAggroComponent::FindConfigForProfile( EEnemyAggroProfile profile ) const
{
	for ( const FAggroProfileConfig& contestant : ProfileConfigs )
	{
		if ( contestant.Profile == profile )
			return &contestant;
	}
	return nullptr;
}

bool UEnemyAggroComponent::CandidatePassesFilters( const FAggroProfileConfig& config, ABuilding* candidate ) const
{
	if ( !candidate )
		return false;

	// can be extend with class checks
	if ( config.bIgnoreEconomyBuildings )
	{
		if ( candidate->IsA( AResourceBuilding::StaticClass() ) )
		{
			return false;
		}
	}

	if ( config.bIgnoreTowers )
	{
		if ( candidate->IsA( ADefensiveBuilding::StaticClass() ) )
		{
			return false;
		}
	}

	return true;
}

int32 UEnemyAggroComponent::GetPriorityIndexForCandidate( const FAggroProfileConfig& config, ABuilding* candidate )
    const
{
	if ( !candidate )
		return INT32_MAX;
	if ( config.PriorityClasses.Num() == 0 )
		return 0;

	for ( int32 i = 0; i < config.PriorityClasses.Num(); ++i )
	{
		const TSubclassOf<ABuilding>& cls = config.PriorityClasses[i];
		if ( !cls )
			continue;
		if ( candidate->IsA( cls ) )
			return i;
	}
	return INT32_MAX;
}

void UEnemyAggroComponent::UpdateOverlapSphereRadius()
{
	if ( !OverlapSphereComponent )
		return;

	const FAggroProfileConfig* config = FindConfigForProfile( AggroProfile );
	if ( !config )
		return;

	const float radiusCm = CellsToCm( config->AggroRadiusCells );
	OverlapSphereComponent->SetSphereRadius( radiusCm, true );
}

void UEnemyAggroComponent::UpdateAggroTarget()
{
	AActor* owner = GetOwner();
	if ( !owner )
		return;

	AUnit* ownerUnit = Cast<AUnit>( owner );
	if ( !ownerUnit )
	{
		UE_LOG( LogTemp, Warning, TEXT( "EnemyAggroComponent: Owner is not AUnit, cannot set FollowedTarget" ) );
		return;
	}

	const FAggroProfileConfig* config = FindConfigForProfile( AggroProfile );
	if ( !config )
	{
		ownerUnit->SetFollowedTarget( nullptr );
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		ownerUnit->SetFollowedTarget( nullptr );
		return;
	}

	const FVector myLoc = owner->GetActorLocation();
	const float radiusCm = CellsToCm( config->AggroRadiusCells );

	if ( bDebugDraw )
	{
		DrawDebugSphere( world, myLoc, radiusCm, 24, FColor::Purple, false, 2.5f, 0, 4.0f );
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	objectTypes.Add( UEngineTypes::ConvertToObjectType( ECC_Pawn ) ); // search pawns

	TArray<AActor*> ignoredActors;
	ignoredActors.Add( owner );

	TArray<AActor*> overlappedActors;
	UKismetSystemLibrary::SphereOverlapActors( world, myLoc, radiusCm, objectTypes, ABuilding::StaticClass(), ignoredActors, overlappedActors );

	int32 bestPriority = INT32_MAX;
	float bestDistSq = TNumericLimits<float>::Max();
	ABuilding* bestBuilding = nullptr;

	for ( AActor* actor : overlappedActors )
	{
		if ( !actor )
			continue;

		ABuilding* B = Cast<ABuilding>( actor );
		if ( !B )
			continue;

		if ( !CandidatePassesFilters( *config, B ) )
			continue;

		const int32 pri = GetPriorityIndexForCandidate( *config, B );
		if ( pri == INT32_MAX )
			continue;

		const float distSq = FVector::DistSquared( myLoc, B->GetActorLocation() );

		if ( pri < bestPriority || ( pri == bestPriority && distSq < bestDistSq ) )
		{
			bestPriority = pri;
			bestDistSq = distSq;
			bestBuilding = B;
		}
	}

	// set owners FollowedTarget
	ownerUnit->SetFollowedTarget( bestBuilding );

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