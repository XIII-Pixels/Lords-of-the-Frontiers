#include "Building/Construction/BuildManager.h"

#include "Building/Building.h"
#include "Building/Construction/BuildPreviewActor.h"
#include "Building/Construction/BuildingPlacementUtils.h"
#include "Building/DefensiveBuilding.h"
#include "Components/StaticMeshComponent.h" // <-- новое
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Grid/GridManager.h"
#include "Grid/GridVisualizer.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

ABuildManager::ABuildManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ABuildManager::BeginPlay()
{
	Super::BeginPlay();

	if ( !GridVisualizer_ )
	{
		if ( UWorld* world = GetWorld() )
		{
			GridVisualizer_ =
			    Cast<AGridVisualizer>( UGameplayStatics::GetActorOfClass( world, AGridVisualizer::StaticClass() ) );
		}
	}

	if ( !GridManager_ )
	{
		if ( UWorld* world = GetWorld() )
		{
			GridManager_ =
			    Cast<AGridManager>( UGameplayStatics::GetActorOfClass( world, AGridManager::StaticClass() ) );
		}
	}
}

void ABuildManager::Tick( const float deltaSeconds )
{
	Super::Tick( deltaSeconds );

	if ( !bIsPlacing_ )
	{
		return;
	}

	UpdateHoveredCell();
}

void ABuildManager::StartPlacingBuilding( TSubclassOf<ABuilding> buildingClass )
{
	if ( !*buildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "StartPlacingBuilding: buildingClass is null" )
			);
		}
		return;
	}

	if ( !GridManager_ || !GridVisualizer_ )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "StartPlacingBuilding: GridManager or GridVisualizer is null" )
			);
		}
		return;
	}

	CurrentBuildingClass_ = buildingClass;
	bIsPlacing_ = true;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( true );

	if ( !PreviewActor_ )
	{
		if ( UWorld* world = GetWorld() )
		{
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			UClass* previewClass = PreviewActorClass_ ? *PreviewActorClass_ : ABuildPreviewActor::StaticClass();

			PreviewActor_ = world->SpawnActor<ABuildPreviewActor>(
			    previewClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams
			);

			if ( !PreviewActor_ && GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Failed to spawn BuildPreviewActor" ) );
			}
		}
	}

	if ( PreviewActor_ )
	{
		const ABuilding* buildingCDO = CurrentBuildingClass_->GetDefaultObject<ABuilding>();

		if ( buildingCDO )
		{
			const UStaticMeshComponent* buildingMeshComp = buildingCDO->FindComponentByClass<UStaticMeshComponent>();

			if ( buildingMeshComp )
			{
				UStaticMesh* buildingMesh = buildingMeshComp->GetStaticMesh();
				if ( buildingMesh )
				{
					PreviewActor_->SetPreviewMesh( buildingMesh );
				}
			}
		}

		PreviewActor_->SetActorHiddenInGame( false );
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Green, TEXT( "Building mode started" ) );
	}
}

void ABuildManager::CancelPlacing()
{
	if ( bIsRelocating_ && RelocatedBuilding_ )
	{
		RelocatedBuilding_->SetActorHiddenInGame( false );
		RelocatedBuilding_->SetActorEnableCollision( true );

		if ( GridVisualizer_ )
		{
			FVector originalLocation;
			if ( GridVisualizer_->GetCellWorldCenter( OriginalCellCoords_, originalLocation ) )
			{
				RelocatedBuilding_->SetActorLocation( originalLocation );
			}
		}
	}

	bIsPlacing_ = false;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( false );

	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( true );
	}

	bIsRelocating_ = false;
	RelocatedBuilding_ = nullptr;
	OriginalCellCoords_ = FIntPoint( -1, -1 );

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Yellow, TEXT( "Building / relocating mode cancelled" ) );
	}
}

void ABuildManager::DemolitionsPlacing( ABuilding* building )
{
	GridManager_->ClearOccupant( building );

	building->Destroy();
}

void ABuildManager::ConfirmPlacing()
{

	if ( !bIsPlacing_ )
	{
		return;
	}

	if ( !GridVisualizer_ || !GridManager_ )
	{
		return;
	}

	if ( !bHasValidCell_ || !bCanBuildHere_ )
	{
		const bool bAllowOriginalCell =
		    bIsRelocating_ && RelocatedBuilding_ && ( CurrentCellCoords_ == OriginalCellCoords_ );

		if ( !bAllowOriginalCell )
		{
			return;
		}
	}

	FVector cellWorldLocation;
	if ( !GridVisualizer_->GetCellWorldCenter( CurrentCellCoords_, cellWorldLocation ) )
	{
		return;
	}

	if ( !bIsRelocating_ || !RelocatedBuilding_ )
	{
		UWorld* world = GetWorld();
		if ( !world )
		{
			return;
		}

		ABuilding* newBuilding = BuildingPlacementUtils::PlaceBuilding(
		    world, CurrentBuildingClass_, cellWorldLocation, GridManager_, CurrentCellCoords_
		);

		if ( !newBuilding )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Red, TEXT( "Failed to place building" ) );
			}
			return;
		}

		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Green, TEXT( "Building placed" ) );
		}

		return;
	}

	FGridCell* oldCell = GridManager_->GetCell( OriginalCellCoords_.X, OriginalCellCoords_.Y );
	FGridCell* newCell = GridManager_->GetCell( CurrentCellCoords_.X, CurrentCellCoords_.Y );

	if ( !newCell )
	{
		return;
	}
	
	RelocatedBuilding_->SetActorLocation( cellWorldLocation );
	RelocatedBuilding_->SetActorHiddenInGame( false );
	RelocatedBuilding_->SetActorEnableCollision( true );

	newCell->bIsOccupied = true;
	newCell->Occupant = RelocatedBuilding_;

	if ( oldCell && ( CurrentCellCoords_ != OriginalCellCoords_ ) )
	{

		if ( oldCell->Occupant.Get() == RelocatedBuilding_ )
		{
			oldCell->bIsOccupied = false;
			oldCell->Occupant.Reset();
		}
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Green, TEXT( "Building relocated" ) );
	}


	bIsPlacing_ = false;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( false );

	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( true );
	}

	bIsRelocating_ = false;
	RelocatedBuilding_ = nullptr;
	OriginalCellCoords_ = FIntPoint( -1, -1 );
}

void ABuildManager::UpdateHoveredCell()
{
	if ( !GridManager_ || !GridVisualizer_ )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	APlayerController* playerController = world->GetFirstPlayerController();
	if ( !playerController )
	{
		return;
	}

	const FBuildingPlacementResult placementResult =
	    BuildingPlacementUtils::FindCellUnderCursor( playerController, GridManager_, GridVisualizer_ );

	bHasValidCell_ = placementResult.bHitValidGrid;

	if ( !bHasValidCell_ )
	{
		if ( PreviewActor_ )
		{
			PreviewActor_->SetActorHiddenInGame( true );
		}

		// HideAllWallPreviews();
		return;
	}

	CurrentCellCoords_ = placementResult.CellCoords;
	bCanBuildHere_ = BuildingPlacementUtils::CanBuildAtCell( GridManager_, CurrentCellCoords_ );

	UpdatePreviewVisual( placementResult.CellWorldLocation, bCanBuildHere_ );

	// UpdateWallPreviews( CurrentCellCoords_, placementResult.CellWorldLocation );
}

void ABuildManager::UpdatePreviewVisual( const FVector& worldLocation, const bool bCanBuild )
{
	if ( !PreviewActor_ )
	{
		return;
	}

	PreviewActor_->SetActorHiddenInGame( false );
	PreviewActor_->SetActorLocation( worldLocation );
	PreviewActor_->SetCanBuild( bCanBuild );
}

static const FIntPoint Offsets4[] = {
    FIntPoint( 0, 1 ),  // вверх
    FIntPoint( 0, -1 ), // вниз
    FIntPoint( -1, 0 ), // влево
    FIntPoint( 1, 0 ),  // вправо
};

TArray<TWeakObjectPtr<ABuilding>> ABuildManager::GetBuildingsNearby( FIntPoint cellCoords )
{
	TArray<TWeakObjectPtr<ABuilding>> tempBuild;
	for ( const FIntPoint& delta : Offsets4 )
	{
		const FIntPoint neighborCoords = cellCoords + delta;

		if ( !GridManager_->IsValidCoords( neighborCoords.X, neighborCoords.Y ) )
		{
			continue;
		}

		const FGridCell* neighborCell;
		neighborCell = GridManager_->GetCell( neighborCoords.X, neighborCoords.Y );

		tempBuild.Add( neighborCell->Occupant );
	}
	return tempBuild;
}

void ABuildManager::HideAllWallPreviews()
{
	for ( auto preview : WallSegmentPreviewActors_ )
	{
		preview->SetActorHiddenInGame( true );
	}
}

void ABuildManager::StartRelocatingBuilding( ABuilding* buildingToMove )
{
	if ( !buildingToMove )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "StartRelocatingBuilding: buildingToMove is null" )
			);
		}
		return;
	}

	if ( !GridManager_ || !GridVisualizer_ )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "StartRelocatingBuilding: GridManager or GridVisualizer is null" )
			);
		}
		return;
	}

	if ( bIsPlacing_ )
	{
		CancelPlacing();
	}

	FIntPoint foundCoords( -1, -1 );
	bool bFound = false;

	const int32 width = GridManager_->GetGridWidth();
	const int32 height = GridManager_->GetGridHeight();

	for ( int32 y = 0; y < height && !bFound; ++y )
	{
		for ( int32 x = 0; x < width; ++x )
		{
			FGridCell* cell = GridManager_->GetCell( x, y );
			if ( !cell )
			{
				continue;
			}

			if ( cell->Occupant.Get() == buildingToMove )
			{
				foundCoords = FIntPoint( x, y );
				bFound = true;
				break;
			}
		}
	}

	if ( !bFound )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "StartRelocatingBuilding: cannot find cell for building" )
			);
		}
		return;
	}

	RelocatedBuilding_ = buildingToMove;
	OriginalCellCoords_ = foundCoords;
	bIsRelocating_ = true;

	RelocatedBuilding_->SetActorHiddenInGame( true );
	RelocatedBuilding_->SetActorEnableCollision( false );

	CurrentBuildingClass_ = buildingToMove->GetClass();
	bIsPlacing_ = true;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( true );

	if ( !PreviewActor_ )
	{
		if ( UWorld* world = GetWorld() )
		{
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			UClass* previewClass = PreviewActorClass_ ? *PreviewActorClass_ : ABuildPreviewActor::StaticClass();

			PreviewActor_ =
			    world->SpawnActor<ABuildPreviewActor>( FVector::ZeroVector, FRotator::ZeroRotator, spawnParams );

			if ( !PreviewActor_ && GEngine )
			{
				GEngine->AddOnScreenDebugMessage(
				    -1, 2.0f, FColor::Red, TEXT( "StartRelocatingBuilding: failed to spawn BuildPreviewActor" )
				);
			}
		}
	}

	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( false );

	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Cyan, TEXT( "Relocate mode started" ) );
	}
}