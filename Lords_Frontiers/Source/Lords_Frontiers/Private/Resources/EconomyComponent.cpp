#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"

#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/Grid/GridManager.h"
#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"

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

void UEconomyComponent::RegisterBuilding( AResourceBuilding* Building )
{
	if ( Building )
	{
		RegisteredBuildings_.AddUnique( Building );
	}
}

void UEconomyComponent::UnregisterBuilding( AResourceBuilding* Building )
{
	RegisteredBuildings_.Remove( Building );
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

	if ( RegisteredBuildings_.Num() == 0 )
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass( GetWorld(), AActor::StaticClass(), FoundActors );

		int32 BuildingsFound = 0;
		for ( AActor* Actor : FoundActors )
		{
			if ( AResourceBuilding* RB = Cast<AResourceBuilding>( Actor ) )
			{
				RegisterBuilding( RB );
				BuildingsFound++;
			}
		}
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 5.f, FColor::Cyan,
			    FString::Printf( TEXT( "Lazy Discovery: Found %d ResourceBuildings" ), BuildingsFound )
			);
		}
	}

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
		if ( AResourceBuilding* ResBuilding = It->Get() )
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
				SuccessfullyProcessed++;
			}
			else
			{
				UE_LOG( LogTemp, Warning, TEXT( "Economy: Building %s has no Generator!" ), *ResBuilding->GetName() );
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
