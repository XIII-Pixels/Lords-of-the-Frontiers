#include "Building/Construction/BuildManager.h"

#include "Building/Building.h"
#include "Building/Construction/BuildPreviewActor.h"
#include "Building/Construction/BuildingPlacementUtils.h"
#include "Building/DefensiveBuilding.h"
#include "Core/CoreManager.h"
#include "DrawDebugHelpers.h"
#include "Grid/GridManager.h"
#include "Grid/GridVisualizer.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"
#include "UI/BonusNeighborhood/BonusIconsData.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( core->IsInitialized() )
		{
			OnCoreSystemsReady();
		}
		else
		{
			core->OnSystemsReady.AddDynamic( this, &ABuildManager::OnCoreSystemsReady );
		}
	}
}

void ABuildManager::OnCoreSystemsReady()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		CachedResourceManager_ = core->GetResourceManager();
	}
}

void ABuildManager::Tick( const float deltaSeconds )
{
	Super::Tick( deltaSeconds );

	if ( !bIsPlacing_ )
	{
		return;
	}

	// 1) Обновляем клетку под курсором и превью.
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
	GridVisualizer_->ShowGrid();

	CurrentBuildingClass_ = buildingClass;
	bIsPlacing_ = true;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( true );

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	UResourceManager* ResManager = pc ? pc->FindComponentByClass<UResourceManager>() : nullptr;

	if ( ResManager )
	{
		const ABuilding* CDO = buildingClass->GetDefaultObject<ABuilding>();
		if ( !ResManager->CanAfford( CDO->GetBuildingCost() ) )
		{
			if ( GEngine )
				GEngine->AddOnScreenDebugMessage( -1, 3.f, FColor::Red, TEXT( "Not enough resources to build this!" ) );
			return;
		}
	}

	// Спавн превью, если ещё не создано.
	if ( !PreviewActor_ )
	{
		if ( UWorld* world = GetWorld() )
		{
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Берём класс из свойства PreviewActorClass_ или дефолтный.
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

	// Настроим меш превью под выбранное здание.
	if ( PreviewActor_ )
	{
		const ABuilding* buildingCDO = CurrentBuildingClass_->GetDefaultObject<ABuilding>();

		if ( buildingCDO )
		{
			// Берём первый StaticMeshComponent у здания (в т.ч. BP-потомков).
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

	CurrentBuildingClass_ = buildingClass;

	ShowBonusHighlightForBuilding( buildingClass );
}

void ABuildManager::CancelPlacing()
{
	// --- Если шёл перенос здания, просто возвращаем его видимость/коллизию ---
	if ( bIsRelocating_ && RelocatedBuilding_ )
	{
		// Вернуть рендер и коллизию.
		RelocatedBuilding_->SetActorHiddenInGame( false );
		RelocatedBuilding_->SetActorEnableCollision( true );

		// На всякий случай вернуть в центр исходной клетки (если вдруг куда-то
		// сдвинули).
		if ( GridVisualizer_ )
		{
			FVector originalLocation;
			if ( GridVisualizer_->GetCellWorldCenter( OriginalCellCoords_, originalLocation ) )
			{
				RelocatedBuilding_->SetActorLocation( originalLocation );
			}
		}
	}

	// --- Общий сброс режима строительства (и для переноса, и для обычного билда)
	// ---
	bIsPlacing_ = false;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( false );

	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( true );
	}

	// Сбрасываем состояние переноса.
	bIsRelocating_ = false;
	RelocatedBuilding_ = nullptr;
	OriginalCellCoords_ = FIntPoint( -1, -1 );

	if ( GridVisualizer_ )
	{
		GridVisualizer_->HideGrid();
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Yellow, TEXT( "Building / relocating mode cancelled" ) );
	}

	CachedBonusIcons_.Empty();
	OnBonusPreviewUpdated.Broadcast( CachedBonusIcons_ );
	GridVisualizer_->HideBonusHighlight();
}

bool ABuildManager::ValidatePlacement( FVector& outCellWorldLocation ) const
{
	if ( !bIsPlacing_ )
	{
		return false;
	}

	if ( !GridVisualizer_ || !GridManager_ )
	{
		return false;
	}

	if ( !CurrentBuildingClass_ )
	{
		UE_LOG( LogTemp, Error, TEXT( "ValidatePlacement: CurrentBuildingClass_ is null" ) );
		return false;
	}

	if ( !bHasValidCell_ || !bCanBuildHere_ )
	{
		const bool bAllowOriginalCell =
		    bIsRelocating_ && RelocatedBuilding_ && ( CurrentCellCoords_ == OriginalCellCoords_ );

		if ( !bAllowOriginalCell )
		{
			return false;
		}
	}

	if ( !GridVisualizer_->GetCellWorldCenter( CurrentCellCoords_, outCellWorldLocation ) )
	{
		return false;
	}

	return true;
}

bool ABuildManager::TryPlaceNewBuilding( const FVector& cellWorldLocation )
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		return false;
	}

	UResourceManager* resManager = CachedResourceManager_.Get();

	const ABuilding* cdo = CurrentBuildingClass_->GetDefaultObject<ABuilding>();
	check( cdo );

	if ( resManager && !resManager->CanAfford( cdo->GetBuildingCost() ) )
	{
		DebugMessage( FColor::Red, TEXT( "Resources ran out!" ), 2.f );
		return false;
	}

	ABuilding* newBuilding = BuildingPlacementUtils::PlaceBuilding(
	    world, CurrentBuildingClass_, cellWorldLocation, GridManager_, CurrentCellCoords_
	);

	if ( !newBuilding )
	{
		DebugMessage( FColor::Red, TEXT( "Failed to place building" ) );
		return false;
	}

	if ( resManager )
	{
		resManager->SpendResources( cdo->GetBuildingCost() );
	}
	RecalculateBonusesAroundBuilding( newBuilding, CurrentCellCoords_ );
	ShowBonusHighlightForBuilding( CurrentBuildingClass_ );
	DebugMessage( FColor::Green, TEXT( "Building placed" ) );
	return true;
}

void ABuildManager::RelocateExistingBuilding( const FVector& cellWorldLocation )
{
	if ( !RelocatedBuilding_ || !GridManager_ )
	{
		return;
	}

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

	if ( CurrentCellCoords_ != OriginalCellCoords_ )
	{

		FGridCell* oldCell = GridManager_->GetCell( OriginalCellCoords_.X, OriginalCellCoords_.Y );

		if ( oldCell && oldCell->Occupant.Get() == RelocatedBuilding_ )
		{
			oldCell->bIsOccupied = false;
			oldCell->Occupant.Reset();
		}
		constexpr int32 MaxBonusRadius = 5;
		RecalculateBonusesAroundBuilding( RelocatedBuilding_, CurrentCellCoords_ );
		RecalculateBonusesFromNeighbors( MaxBonusRadius, OriginalCellCoords_ );
	}

	DebugMessage( FColor::Green, TEXT( "Building relocated" ) );
}

void ABuildManager::ResetPlacementState()
{
	bIsPlacing_ = false;
	bHasValidCell_ = false;
	bCanBuildHere_ = false;

	PrimaryActorTick.SetTickFunctionEnable( false );

	if ( PreviewActor_ )
	{
		PreviewActor_->SetActorHiddenInGame( true );
	}

	if ( GridVisualizer_ )
	{
		GridVisualizer_->HideGrid();
	}

	bIsRelocating_ = false;
	RelocatedBuilding_ = nullptr;
	OriginalCellCoords_ = FIntPoint( -1, -1 );
}

void ABuildManager::ConfirmPlacing()
{
	FVector cellWorldLocation;
	if ( !ValidatePlacement( cellWorldLocation ) )
	{
		return;
	}

	if ( !bIsRelocating_ || !RelocatedBuilding_ )
	{
		TryPlaceNewBuilding( cellWorldLocation );
		return;
	}

	RelocateExistingBuilding( cellWorldLocation );
	ResetPlacementState();
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

	const bool bCellChanged = ( CurrentCellCoords_ != placementResult.CellCoords );
	CurrentCellCoords_ = placementResult.CellCoords;
	bCanBuildHere_ = BuildingPlacementUtils::CanBuildAtCell( GridManager_, CurrentCellCoords_ );
	UpdatePreviewVisual( placementResult.CellWorldLocation, bCanBuildHere_ );

	if ( bCellChanged )
	{
		if ( bCanBuildHere_ )
		{
			TArray<FBonusIconData> rawIcons = CollectBonusPreview( CurrentBuildingClass_, CurrentCellCoords_ );
			CachedBonusIcons_ = AggregateBonusIcons( rawIcons );
		}
		else
		{
			CachedBonusIcons_.Empty();
		}
		OnBonusPreviewUpdated.Broadcast( CachedBonusIcons_ );
	}
	// UpdateWallPreviews( CurrentCellCoords_, placementResult.CellWorldLocation
	// );
}

void ABuildManager::UpdatePreviewVisual( const FVector& worldLocation, const bool bCanBuild )
{
	if ( !PreviewActor_ )
	{
		return;
	}

	PreviewActor_->SetActorHiddenInGame( false );
	PreviewActor_->SetActorLocation( worldLocation );
	PreviewActor_->SetCanBuild( bCanBuild ); // метод внутри ABuildPreviewActor (зелёный/красный и т.п.)
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
			    -1, 2.0f, FColor::Red,
			    TEXT(
			        "StartRelocatingBuilding: GridManager or GridVisualizer is "
			        "null"
			    )
			);
		}
		return;
	}

	if ( bIsPlacing_ )
	{
		CancelPlacing();
	}

	if ( GridVisualizer_ )
	{
		GridVisualizer_->ShowGrid();
	}
	// 2) Находим клетку, в которой сейчас стоит это здание.
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

			// В этой клетке стоит наш buildingToMove?
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

	// 3) Сохраняем состояние переноса.
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

	// 5) Спавним/показываем превью как при обычном строительстве.
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
		// Если у PreviewActor есть метод вроде SetSourceClass /
		// SetPreviewMeshFromClass, можно здесь передать CurrentBuildingClass_,
		// чтобы превью подстроилось под здание.
		// PreviewActor_->InitFromBuildingClass( CurrentBuildingClass_ );
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.5f, FColor::Cyan, TEXT( "Relocate mode started" ) );
	}
}

void ABuildManager::RecalculateBonusesAroundBuilding( ABuilding* building, const FIntPoint& cellCoords )
{
	if ( !building || !GridManager_ )
	{
		return;
	}

	UBuildingBonusComponent* bonusComponent = building->FindComponentByClass<UBuildingBonusComponent>();
	if ( bonusComponent )
	{
		bonusComponent->RecalculateBonuses( GridManager_, cellCoords );
	}

	RecalculateBonusesFromNeighbors( UBuildingBonusComponent::MaxPossibleBonusRadius, cellCoords );
}

void ABuildManager::RecalculateBonusesFromNeighbors( const int32 MaxBonusRadius, const FIntPoint& cellCoords )
{
	TArray<FGridCell*> neighbors = GridManager_->GetCellsInRadius( cellCoords, MaxBonusRadius );
	for ( FGridCell* cell : neighbors )
	{
		if ( !cell || !cell->Occupant.IsValid() )
		{
			continue;
		}

		ABuilding* neighbor = Cast<ABuilding>( cell->Occupant.Get() );
		if ( !neighbor )
		{
			continue;
		}

		UBuildingBonusComponent* neighborBonus = neighbor->FindComponentByClass<UBuildingBonusComponent>();

		if ( neighborBonus )
		{
			neighborBonus->RecalculateBonuses( GridManager_, cell->GridCoords );
		}
	}
}
// USTRUCT( BlueprintType )
// struct FBonusIconData
//{
//	GENERATED_BODY()
//
//	UPROPERTY()
//	FVector WorldLocation = FVector::ZeroVector;
//
//	UPROPERTY()
//	TObjectPtr<UTexture2D> Icon = nullptr;
//
//	UPROPERTY()
//	float Value = 0.0f;
//
//	UPROPERTY()
//	EBonusCategory Category = EBonusCategory::Production;
// };

TArray<FBonusIconData>
ABuildManager::CollectBonusPreview( TSubclassOf<ABuilding> buildingClass, const FIntPoint& cellCoords )
{
	TArray<FBonusIconData> result;

	if ( !buildingClass || !GridManager_ )
	{
		return result;
	}

	FVector worldLocation;
	if ( !GridManager_->GetCellWorldCenter( cellCoords, worldLocation ) )
	{
		return result;
	}
	const ABuilding* cdo = buildingClass->GetDefaultObject<ABuilding>();
	UBuildingBonusComponent* bonusComp = UBuildingBonusComponent::FindInBlueprintClass( buildingClass );

	UE_LOG( LogTemp, Warning, TEXT( "=== CollectBonusPreview ===" ) );
	UE_LOG( LogTemp, Warning, TEXT( "Building class: %s" ), *buildingClass->GetName() );
	UE_LOG( LogTemp, Warning, TEXT( "BonusComp found: %s" ), bonusComp ? TEXT( "YES" ) : TEXT( "NO" ) );

	if ( bonusComp )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Part 1: entries count: %d" ), bonusComp->GetBonusEntries().Num() );
		const TArray<FBuildingBonusEntry>& entries = bonusComp->GetBonusEntries();

		for ( int32 i = 0; i < entries.Num(); ++i )
		{
			TArray<FGridCell*> neighbors = GridManager_->GetCellsInRadius( cellCoords, entries[i].Radius );

			for ( const FGridCell* cell : neighbors )
			{
				ABuilding* occupant = Cast<ABuilding>( cell->Occupant.Get() );
				if ( occupant && occupant->IsA( entries[i].SourceBuildingClass ) )
				{
					FBonusIconData iconData = bonusComp->GetInfoSingleBonus( i, worldLocation );
					iconData.BuildingIcon = cdo->BuildingIcon;
					iconData.CellCoords = cellCoords;
					result.Add( iconData );
				}
			}
		}
	}
	UE_LOG( LogTemp, Warning, TEXT( "Part 1 result count: %d" ), result.Num() );
	CollectBonusFromNeighbors( cellCoords, result, cdo );
	UE_LOG( LogTemp, Warning, TEXT( "After Part 2 total count: %d" ), result.Num() );

	return result;
};

void ABuildManager::CollectBonusFromNeighbors(
    const FIntPoint& myCellCoords, TArray<FBonusIconData>& result, const ABuilding* cdo
)
{
	if ( !GridManager_ || !cdo )
	{
		return;
	}

	TArray<FGridCell*> neighborsCells =
	    GridManager_->GetCellsInRadius( myCellCoords, UBuildingBonusComponent::MaxPossibleBonusRadius );

	if ( neighborsCells.IsEmpty() )
	{
		return;
	}

	UE_LOG( LogTemp, Warning, TEXT( "=== CollectBonusFromNeighbors ===" ) );
	UE_LOG( LogTemp, Warning, TEXT( "Neighbors found: %d" ), neighborsCells.Num() );

	for ( const FGridCell* cell : neighborsCells )
	{
		if ( !cell || !cell->bIsOccupied )
		{
			continue;
		}

		ABuilding* neighbor = Cast<ABuilding>( cell->Occupant.Get() );
		if ( !neighbor )
		{
			continue;
		}

		UBuildingBonusComponent* bonusComp = neighbor->FindComponentByClass<UBuildingBonusComponent>();
		if ( !bonusComp )
		{
			continue;
		}

		const TArray<FBuildingBonusEntry>& entries = bonusComp->GetBonusEntries();

		for ( int32 i = 0; i < entries.Num(); ++i )
		{

			const int32 dx = FMath::Abs( myCellCoords.X - cell->GridCoords.X );
			const int32 dy = FMath::Abs( myCellCoords.Y - cell->GridCoords.Y );
			const int32 distance = FMath::Max( dx, dy );

			if ( distance > entries[i].Radius )
			{
				continue;
			}
			UE_LOG(
			    LogTemp, Warning, TEXT( "Neighbor: %s, has BonusComp: %s, entries: %d" ), *neighbor->GetName(),
			    bonusComp ? TEXT( "YES" ) : TEXT( "NO" ), bonusComp ? bonusComp->GetBonusEntries().Num() : 0
			);

			if ( cdo->IsA( entries[i].SourceBuildingClass ) )
			{
				UE_LOG(
				    LogTemp, Warning, TEXT( "  Entry %d: SourceClass=%s, CDO IsA=%s" ), i,
				    entries[i].SourceBuildingClass ? *entries[i].SourceBuildingClass->GetName() : TEXT( "None" ),
				    cdo->IsA( entries[i].SourceBuildingClass ) ? TEXT( "YES" ) : TEXT( "NO" )
				);
				FVector neighborWorldLocation;
				if ( GridManager_->GetCellWorldCenter( cell->GridCoords, neighborWorldLocation ) )
				{
					FBonusIconData iconData = bonusComp->GetInfoSingleBonus( i, neighborWorldLocation );
					iconData.BuildingIcon = neighbor->BuildingIcon;
					iconData.CellCoords = cell->GridCoords;
					result.Add( iconData );
				}
			}
		}
	}
}

void ABuildManager::AppendMatchingBonuses(
    UBuildingBonusComponent* bonusComp, TSubclassOf<ABuilding> matchClass, const FVector& iconWorldLocation,
    TArray<FBonusIconData>& OutResult
)
{
	if ( !bonusComp || !matchClass )
	{
		return;
	}
	const TArray<FBuildingBonusEntry>& Entries = bonusComp->GetBonusEntries();
	for ( int32 i = 0; i < Entries.Num(); ++i )
	{
		if ( matchClass->IsChildOf( Entries[i].SourceBuildingClass ) )
		{
			OutResult.Add( bonusComp->GetInfoSingleBonus( i, iconWorldLocation ) );
		}
	}
}

TArray<FBonusIconData> ABuildManager::AggregateBonusIcons( const TArray<FBonusIconData>& rawIcons )
{
	TMap<FString, FBonusIconData> grouped;

	for ( const FBonusIconData& icon : rawIcons )
	{
		FString key = FString::Printf(
		    TEXT( "%d_%d_%d_%d" ), icon.CellCoords.X, icon.CellCoords.Y, static_cast<uint8>( icon.ResourceType ),
		    static_cast<uint8>( icon.StatType )
		);

		if ( FBonusIconData* existing = grouped.Find( key ) )
		{
			float signedValue = ( icon.Category == EBonusCategory::Maintenance ) ? -icon.Value : icon.Value;
			existing->Value += signedValue;
		}
		else
		{
			FBonusIconData newIcon = icon;
			if ( icon.Category == EBonusCategory::Maintenance )
			{
				newIcon.Value = -icon.Value;
			}
			grouped.Add( key, newIcon );
		}
	}

	TArray<FBonusIconData> result;
	for ( auto& kvp : grouped )
	{
		if ( !FMath::IsNearlyZero( kvp.Value.Value ) )
		{
			result.Add( kvp.Value );
		}
	}
	return result;
}

void ABuildManager::ShowBonusHighlightForBuilding( TSubclassOf<ABuilding> buildingClass )
{
	if ( !GridVisualizer_ || !GridManager_ || !buildingClass )
	{
		return;
	}

	TArray<FIntPoint> cells = UBuildingBonusComponent::FindBonusCells( buildingClass, GridManager_ );

	UMaterialInterface* highlightMat = BonusIconsData ? BonusIconsData->HighlightMaterial : nullptr;

	if ( cells.Num() > 0 && highlightMat )
	{
		GridVisualizer_->ShowBonusHighlight( cells, highlightMat );
	}
	else
	{
		GridVisualizer_->HideBonusHighlight();
	}
}

void ABuildManager::DebugMessage( const FColor& color, const FString& message, float duration )
{
#if !UE_BUILD_SHIPPING
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, duration, color, message );
	}
#endif
}