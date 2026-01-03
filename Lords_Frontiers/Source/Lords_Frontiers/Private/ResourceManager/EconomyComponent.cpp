#include "Lords_Frontiers/Public/ResourceManager/EconomyComponent.h"
#include "Lords_Frontiers/Public/Grid/GridManager.h"
#include "Lords_Frontiers/Public/Grid/GridCell.h"
#include "Lords_Frontiers/Public/ResourceManager/ResourceManager.h"
#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/ResourceManager/ResourceGenerator.h"
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
	//find GridManager in world
	GridManager_ = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	//find ResourceManager
	AActor* Owner = GetOwner();
	if ( Owner )
	{
		ResourceManager_ = Owner->FindComponentByClass<UResourceManager>();
	}

	if ( !GridManager_ ) UE_LOG( LogTemp, Warning, TEXT( "EconomyComponent: GridManager not found!" ) );
	if ( !ResourceManager_ ) UE_LOG( LogTemp, Warning, TEXT( "EconomyComponent: ResourceManager not found on owner!" ) );
}

void UEconomyComponent::CollectGlobalResources()
{
    if (!GridManager_ || !ResourceManager_)
    {
        FindSystems();
        if (!GridManager_ || !ResourceManager_) {
            return;
        }
    }

    TMap<EResourceType, int32> CollectedTotals;
    const int32 Width = GridManager_->GetGridWidth();
    const int32 Height = GridManager_->GetGridHeight();

    for (int32 y = 0; y < Height; ++y)
    {
        for (int32 x = 0; x < Width; ++x)
        {
            FGridCell* Cell = GridManager_->GetCell(x, y);
            if (Cell && Cell->bIsOccupied)
            {
                if (!Cell->Occupant.IsValid())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cell [%d,%d] is occupied but Occupant is NULL"), x, y);
                    continue;
                }

                AActor* Actor = Cell->Occupant.Get();

                AResourceBuilding* ResBuilding = Cast<AResourceBuilding>(Actor);
                if (!ResBuilding)
                {
                    UE_LOG(LogTemp, Error, TEXT("Actor %s is NOT a ResourceBuilding! (Cast Failed)"), *Actor->GetName());
                    continue;
                }

                UResourceGenerator* Gen = ResBuilding->GetResourceGenerator();
                if (!Gen)
                {
                    UE_LOG(LogTemp, Error, TEXT("Building %s has NO ResourceGenerator component!"), *Actor->GetName());
                    continue;
                }

                EResourceType Type = Gen->GetResourceType();
                int32 Qty = Gen->GetGenerationQuantity();

                CollectedTotals.FindOrAdd(Type) += Qty;
                UE_LOG(LogTemp, Log, TEXT("Found %d of type %d in building %s"), Qty, (uint8)Type, *Actor->GetName());
            }
        }
    }

    for (const auto& Elem : CollectedTotals)
    {
        ResourceManager_->AddResource(Elem.Key, Elem.Value);
        if (GEngine)
        {
            FString Msg = FString::Printf(TEXT("SUCCESS! Added %d resources"), Elem.Value);
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, Msg);
        }
    }
}