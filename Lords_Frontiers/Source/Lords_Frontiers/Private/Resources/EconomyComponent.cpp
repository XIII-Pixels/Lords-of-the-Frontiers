#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/Grid/GridManager.h"
#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

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

void UEconomyComponent::RegisterBuilding( ABuilding* Building )
{
	if ( Building )
		RegisteredBuildings_.AddUnique( Building );
}

void UEconomyComponent::UnregisterBuilding( ABuilding* Building )
{
	RegisteredBuildings_.Remove( Building );
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
		GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Orange, TEXT( "Economy: Collecting..." ) );
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

	TMap<EResourceType, int32> CollectedTotals;
	int32 SuccessfullyProcessed = 0;

	for ( auto It = RegisteredBuildings_.CreateIterator(); It; ++It )
	{
		if ( ABuilding* B = It->Get() )
		{
			if ( AResourceBuilding* ResBuilding = Cast<AResourceBuilding>( B ) )
			{
				UResourceGenerator* Gen = ResBuilding->GetResourceGenerator();
				if ( Gen )
				{
					for ( const auto& Elem : Gen->GetTotalProduction() )
					{
						if ( Elem.Key != EResourceType::None && Elem.Value > 0 )
						{
							CollectedTotals.FindOrAdd( Elem.Key ) += Elem.Value;
						}
					}
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

	for ( const auto& Elem : CollectedTotals )
	{
		ResourceManager_->AddResource( Elem.Key, Elem.Value );
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 5.f, FColor::Green, FString::Printf( TEXT( "Added %d of type %d" ), Elem.Value, (uint8) Elem.Key )
			);
		}
	}
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