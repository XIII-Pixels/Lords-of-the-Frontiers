#include "Building/Construction/BuildingConstructionComponent.h"

#include "Building/Building.h"
#include "Building/Construction/BuildPreviewActor.h"
#include "Grid/GridManager.h"
#include "Grid/GridVisualizer.h"

#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UBuildingConstructionComponent::UBuildingConstructionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UBuildingConstructionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuildingConstructionComponent::Initialize( AGridManager* inGridManager, AGridVisualizer* inGridVisualizer )
{
	GridManager_ = inGridManager;
	GridVisualizer_ = inGridVisualizer;
}

void UBuildingConstructionComponent::StartPlacingBuilding( TSubclassOf<ABuilding> buildingClass )
{
	if ( !*buildingClass )
	{
		// Nothing to build.
		return;
	}

	CurrentBuildingClass_ = buildingClass;
	bIsPlacing_ = true;

	// Spawn preview actor if needed.
	if ( !PreviewActor_ )
	{
		if ( UWorld* world = GetWorld() )
		{
			const FActorSpawnParameters spawnParams;
			PreviewActor_ =
			    world->SpawnActor<ABuildPreviewActor>( FVector::ZeroVector, FRotator::ZeroRotator, spawnParams );
		}
	}

	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( false );
	}
}

void UBuildingConstructionComponent::CancelPlacing()
{
	bIsPlacing_ = false;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	// Hide preview but keep it for reuse.
	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( true );
	}
}

void UBuildingConstructionComponent::ConfirmPlacing()
{
	if ( !bIsPlacing_ || !bHasValidCell_ || !bCanBuildHere_ )
	{
		return;
	}

	if ( !GridVisualizer_ )
	{
		return;
	}

	FVector cellWorldLocation;
	if ( !GridVisualizer_->GetCellWorldCenter( CurrentCellCoords_, cellWorldLocation ) )
	{
		return;
	}

	PlaceBuilding( CurrentCellCoords_, cellWorldLocation );
}

void UBuildingConstructionComponent::TickComponent(
    const float deltaTime, const ELevelTick tickType, FActorComponentTickFunction* thisTickFunction
)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	// Do nothing if building mode is not active.
	if ( !bIsPlacing_ )
	{
		return;
	}

	UpdateHoveredCell();
}

void UBuildingConstructionComponent::UpdateHoveredCell()
{
	if ( !GridManager_ || !GridVisualizer_ )
	{
		return;
	}

	FIntPoint cellCoords;
	FVector cellWorldLocation;

	bHasValidCell_ = FindCellUnderCursor( cellCoords, cellWorldLocation );
	if ( !bHasValidCell_ )
	{
		if ( PreviewActor_ )
		{
			PreviewActor_->SetActorHiddenInGame( true );
		}
		return;
	}

	CurrentCellCoords_ = cellCoords;
	bCanBuildHere_ = CanBuildAtCell( cellCoords );

	UpdatePreviewVisual( cellWorldLocation, bCanBuildHere_ );
}

bool UBuildingConstructionComponent::FindCellUnderCursor( FIntPoint& outCellCoords, FVector& outCellWorldLocation )
    const
{
	if ( !GridManager_ || !GridVisualizer_ )
	{
		return false;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return false;
	}

	APlayerController* pc = world->GetFirstPlayerController();
	if ( !pc )
	{
		return false;
	}

	FHitResult hit;
	// Trace under cursor by Visibility channel (can be replaced with custom
	// ground channel).
	if ( !pc->GetHitResultUnderCursorByChannel(
	         UEngineTypes::ConvertToTraceType( ECollisionChannel::ECC_Visibility ), false, hit
	     ) )
	{
		return false;
	}

	const float cellSize = GridManager_->GetCellSize();
	const FVector origin = GridManager_->GetActorLocation();

	if ( cellSize <= KINDA_SMALL_NUMBER )
	{
		return false;
	}

	const float localX = hit.Location.X - origin.X;
	const float localY = hit.Location.Y - origin.Y;

	const int32 gridX = FMath::FloorToInt( localX / cellSize );
	const int32 gridY = FMath::FloorToInt( localY / cellSize );

	const FGridCell* cell = GridManager_->GetCell( gridX, gridY );
	if ( !cell )
	{
		return false;
	}

	outCellCoords.X = gridX;
	outCellCoords.Y = gridY;

	// Use visualizer to get cell center (shared logic with grid rendering).
	if ( !GridVisualizer_->GetCellWorldCenter( outCellCoords, outCellWorldLocation ) )
	{
		return false;
	}

	return true;
}

bool UBuildingConstructionComponent::CanBuildAtCell( const FIntPoint& cellCoords ) const
{
	if ( !GridManager_ )
	{
		return false;
	}

	const FGridCell* cell = GridManager_->GetCell( cellCoords.X, cellCoords.Y );
	if ( !cell )
	{
		return false;
	}

	// Base conditions: cell is buildable and not occupied.
	if ( !cell->bIsBuildable || cell->bIsOccupied )
	{
		return false;
	}

	// Here you can later add extra conditions (resources, neighbors etc.).
	return true;
}

void UBuildingConstructionComponent::UpdatePreviewVisual( const FVector& worldLocation, const bool bCanBuild )
{
	if ( !PreviewActor_ )
	{
		return;
	}

	PreviewActor_->SetActorHiddenInGame( false );
	PreviewActor_->SetActorLocation( worldLocation );
	PreviewActor_->SetCanBuild( bCanBuild );
}

void UBuildingConstructionComponent::PlaceBuilding( const FIntPoint& cellCoords, const FVector& worldLocation )
{
	if ( !GridManager_ || !*CurrentBuildingClass_ )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABuilding* building =
	    world->SpawnActor<ABuilding>( CurrentBuildingClass_, FTransform( worldLocation ), spawnParams );

	if ( !building )
	{
		return;
	}

	// Update cell state in grid.
	FGridCell* cell = GridManager_->GetCell( cellCoords.X, cellCoords.Y );
	if ( cell )
	{
		cell->bIsOccupied = true;
		cell->Occupant = building;
	}
}
