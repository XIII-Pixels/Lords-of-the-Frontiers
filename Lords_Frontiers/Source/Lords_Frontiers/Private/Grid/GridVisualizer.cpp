#include "Grid/GridVisualizer.h"

#include "Grid/GridManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AGridVisualizer::AGridVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent( CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) ) );

	BuildableMesh_ = CreateDefaultSubobject<UInstancedStaticMeshComponent>( TEXT( "BuildableMesh" ) );
	BlockedMesh_ = CreateDefaultSubobject<UInstancedStaticMeshComponent>( TEXT( "BlockedMesh" ) );

	BuildableMesh_->SetupAttachment( RootComponent );
	BlockedMesh_->SetupAttachment( RootComponent );

	BuildableMesh_->SetCastShadow( false );
	BlockedMesh_->SetCastShadow( false );

	BuildableMesh_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	BlockedMesh_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );

	BuildableMesh_->SetCollisionResponseToAllChannels( ECR_Ignore );
	BlockedMesh_->SetCollisionResponseToAllChannels( ECR_Ignore );

	BuildableMesh_->SetCollisionResponseToChannel( ECollisionChannel::ECC_GameTraceChannel1, ECR_Block );
	BlockedMesh_->SetCollisionResponseToChannel( ECollisionChannel::ECC_GameTraceChannel1, ECR_Block );
}

void AGridVisualizer::BeginPlay()
{
	Super::BeginPlay();

	if ( !GridManager_ )
	{
		GridManager_ =
		    Cast<AGridManager>( UGameplayStatics::GetActorOfClass( GetWorld(), AGridManager::StaticClass() ) );
	}

	SetActorHiddenInGame( !bGridVisible_ );

	if ( BuildableMesh_ )
	{
		BuildableMesh_->SetVisibility( bGridVisible_, true );
	}
	if ( BlockedMesh_ )
	{
		BlockedMesh_->SetVisibility( bGridVisible_, true );
	}

	if ( bGridVisible_ )
	{
		RefreshGrid();
	}
}

void AGridVisualizer::ShowGrid()
{
	SetGridVisible( true );
}

void AGridVisualizer::HideGrid()
{
	SetGridVisible( false );
}

void AGridVisualizer::SetGridVisible( const bool bVisible )
{
	if ( bGridVisible_ == bVisible )
	{
		return;
	}

	bGridVisible_ = bVisible;

	if ( !BuildableMesh_ || !BlockedMesh_ )
	{
		return;
	}

	if ( !bGridVisible_ )
	{

		BuildableMesh_->ClearInstances();
		BlockedMesh_->ClearInstances();

		BuildableMesh_->SetVisibility( false, true );
		BlockedMesh_->SetVisibility( false, true );
		SetActorHiddenInGame( true );

		return;
	}

	SetActorHiddenInGame( false );
	BuildableMesh_->SetVisibility( true, true );
	BlockedMesh_->SetVisibility( true, true );

	RefreshGrid();
}

void AGridVisualizer::RefreshGrid()
{
	if ( !GridManager_ || !BuildableMesh_ || !BlockedMesh_ )
	{
		return;
	}

	const int32 height = GridManager_->GetGridHeight();

	if ( height <= 0 )
	{
		return;
	}

	const float cellSize = GridManager_->GetCellSize();
	const FVector origin = GridManager_->GetActorLocation();
	const float z = origin.Z + ZOffset_;

	BuildableMesh_->ClearInstances();
	BlockedMesh_->ClearInstances();

	if ( !BuildableMesh_->GetStaticMesh() || !BlockedMesh_->GetStaticMesh() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridVisualizer: StaticMesh not assigned on components" ) );
		return;
	}

	for ( int32 y = 0; y < height; ++y )
	{
		const int32 rowWidth = GridManager_->GetRowWidth( y );

		for ( int32 x = 0; x < rowWidth; ++x )
		{
			const FGridCell* cell = GridManager_->GetCell( x, y );
			if ( !cell )
			{
				continue;
			}

			const float centerX = origin.X + ( static_cast<float>( x ) + 0.5f ) * cellSize;
			const float centerY = origin.Y + ( static_cast<float>( y ) + 0.5f ) * cellSize;

			const FVector location( centerX, centerY, z );

			const FRotator rotation = FRotator::ZeroRotator;

			const float baseScale = cellSize / 100.0f;
			const FVector scale3D( baseScale * MeshScale_ );

			const FTransform instanceTransform( rotation, location, scale3D );

			if ( cell->bIsBuildable )
			{
				BuildableMesh_->AddInstance( instanceTransform );
			}
			else
			{
				BlockedMesh_->AddInstance( instanceTransform );
			}
		}
	}
}

bool AGridVisualizer::GetCellWorldCenter( const FIntPoint& cellCoords, FVector& outLocation ) const
{
	if ( !GridManager_ )
	{
		return false;
	}

	bool success = GridManager_->GetCellWorldCenter( cellCoords, outLocation );
	outLocation.Z += ZOffset_;
	return success;
}
