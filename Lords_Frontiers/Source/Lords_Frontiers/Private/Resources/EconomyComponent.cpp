#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/Grid/GridManager.h"
#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"
#include "Lords_Frontiers/Public/Core/CoreManager.h"

UEconomyComponent::UEconomyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GridManager_ = nullptr;
	ResourceManager_ = nullptr;
}

void UEconomyComponent::BeginPlay()
{
	Super::BeginPlay();
	FindSystems();
}

void UEconomyComponent::FindSystems()
{
	// find GridManager in world
	GridManager_ = Cast<AGridManager>( UGameplayStatics::GetActorOfClass( GetWorld(), AGridManager::StaticClass() ) );

	// find ResourceManager
	AActor* Owner = GetOwner();
	if ( Owner )
	{
		ResourceManager_ = Owner->FindComponentByClass<UResourceManager>();
	}

	if ( !GridManager_ )
		UE_LOG( LogTemp, Warning, TEXT( "EconomyComponent: GridManager not found!" ) );
	if ( !ResourceManager_ )
		UE_LOG( LogTemp, Warning, TEXT( "EconomyComponent: ResourceManager not found on owner!" ) );
}

void UEconomyComponent::RegisterBuilding( ABuilding* building )
{
	if ( building )
		RegisteredBuildings_.AddUnique( building );
}

void UEconomyComponent::UnregisterBuilding( ABuilding* building )
{
	RegisteredBuildings_.Remove( building );
}

void UEconomyComponent::PerformInitialScan()
{
	if ( bInitialScanDone )
		return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ABuilding::StaticClass(), FoundActors );

	for ( AActor* Actor : FoundActors )
	{
		if ( ABuilding* B = Cast<ABuilding>( Actor ) )
		{
			RegisterBuilding( B );
		}
	}

	bInitialScanDone = true;
	UE_LOG(
	    LogTemp, Warning, TEXT( "Economy: Initial Scan completed. Found %d buildings." ), RegisteredBuildings_.Num()
	);
}

void UEconomyComponent::CollectGlobalResources()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Orange, TEXT( "Economy: Collecting resources..." ) );
	}

	if ( !ResourceManager_ )
	{
		FindSystems();
		if ( !ResourceManager_ )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Red, TEXT( "Error: ResourceManager is NULL" ) );
			}
			return;
		}
	}

	PerformInitialScan();

	if ( RegisteredBuildings_.Num() == 0 )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 5.f, FColor::Red, TEXT( "Error: No buildings registered even after discovery!" )
			);
		}
		return;
	}

	TMap<EResourceType, int32> collectedTotals;
	int32 SuccessfullyProcessed = 0;

	for ( auto It = RegisteredBuildings_.CreateIterator(); It; ++It )
	{
		if ( ABuilding* B = It->Get() )
		{
			if ( AResourceBuilding* resBuilding = Cast<AResourceBuilding>( B ) )
			{
				if ( UResourceGenerator* gen = resBuilding->GetResourceGenerator() )
				{
					TMap<EResourceType, int32> productionMap = gen->GetTotalProduction();

					for ( const auto& pair : productionMap )
					{
						if ( pair.Key != EResourceType::None && pair.Value > 0 )
						{
							collectedTotals.FindOrAdd( pair.Key ) += pair.Value;
						}
					}
					++SuccessfullyProcessed;
				}
			}
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
		    -1, 5.f, FColor::Yellow, FString::Printf( TEXT( "Processed: %d buildings" ), SuccessfullyProcessed )
		);
	}

	for ( const auto& Elem : collectedTotals )
	{
		ResourceManager_->AddResource( Elem.Key, Elem.Value );
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 5.f, FColor::Green, FString::Printf( TEXT( "Added %d of type %d" ), Elem.Value, (uint8) Elem.Key )
			);
		}
	}
	OnResourcesCollected.Broadcast();
}

void UEconomyComponent::ApplyMaintenanceCosts()
{
	if ( !ResourceManager_ )
		return;

	PerformInitialScan();

	TMap<EResourceType, int32> TotalCosts;

	for ( auto It = RegisteredBuildings_.CreateIterator(); It; ++It )
	{
		if ( ABuilding* B = It->Get() )
		{
			TMap<EResourceType, int32> Costs = B->GetMaintenanceCost().ToMap();
			for ( const auto& Pair : Costs )
			{
				if ( Pair.Value > 0 )
				{
					TotalCosts.FindOrAdd( Pair.Key ) += Pair.Value;
				}
			}
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	for ( const auto& Pair : TotalCosts )
	{
		ResourceManager_->TrySpendResource( Pair.Key, Pair.Value );
	}
}

void UEconomyComponent::RestoreAllBuildings()
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ABuilding::StaticClass(), foundActors );

	for ( AActor* actor : foundActors )
	{
		if ( ABuilding* b = Cast<ABuilding>( actor ) )
		{
			b->RestoreFromRuins();
		}
	}
}
TMap<EResourceType, int32> UEconomyComponent::ComputePerTurnTotals() const
{
	TMap<EResourceType, int32> PerTurnTotals;

	for ( auto It = RegisteredBuildings_.CreateConstIterator(); It; ++It )
	{
		ABuilding* B = It->Get();
		if ( !B )
		{
			continue;
		}

		if ( AResourceBuilding* RB = Cast<AResourceBuilding>( B ) )
		{
			if ( UResourceGenerator* Gen = RB->GetResourceGenerator() )
			{
				const TMap<EResourceType, int32> Production = Gen->GetTotalProduction();
				for ( const auto& Pair : Production )
				{
					if ( Pair.Key == EResourceType::None )
						continue;
					if ( Pair.Value == 0 )
						continue;

					PerTurnTotals.FindOrAdd( Pair.Key ) += Pair.Value; // add positive or negative
				}
			}
		}

		{
			const TMap<EResourceType, int32> Costs = B->GetMaintenanceCost().ToMap();
			for ( const auto& Cost : Costs )
			{
				if ( Cost.Key == EResourceType::None )
					continue;
				if ( Cost.Value == 0 )
					continue;

				PerTurnTotals.FindOrAdd( Cost.Key ) -= Cost.Value;
			}
		}
	}

	return PerTurnTotals;
}

void UEconomyComponent::UpdatePerTurnEstimates()
{
	if ( !ResourceManager_ )
	{
		FindSystems();
		if ( !ResourceManager_ )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Economy: UpdatePerTurnEstimates: ResourceManager is NULL" ) );
			return;
		}
	}

	PerformInitialScan();

	for ( int32 i = RegisteredBuildings_.Num() - 1; i >= 0; --i )
	{
		if ( !RegisteredBuildings_[i].IsValid() )
		{
			RegisteredBuildings_.RemoveAtSwap( i );
		}
	}

	const TMap<EResourceType, int32> NewPerTurn = ComputePerTurnTotals();

	for ( uint8 t = (uint8) EResourceType::None + 1; t < (uint8) EResourceType::Max; ++t )
	{
		const EResourceType Type = static_cast<EResourceType>( t );
		const int32* Found = NewPerTurn.Find( Type );
		const int32 NewValue = Found ? *Found : 0;
		ResourceManager_->SetResourcePerTurn( Type, NewValue );
	}

#if WITH_EDITOR || UE_BUILD_DEBUG
	// Debug
	FString DebugStr = TEXT( "Economy: Per-turn totals:" );
	for ( const auto& Elem : NewPerTurn )
	{
		DebugStr += FString::Printf( TEXT( " [%d:%+d]" ), (int) Elem.Key, Elem.Value );
	}
	UE_LOG( LogTemp, Log, TEXT( "%s" ), *DebugStr );
#endif
}