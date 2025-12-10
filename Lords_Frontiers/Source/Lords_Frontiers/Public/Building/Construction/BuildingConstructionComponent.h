#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Grid/GridCell.h"

#include "BuildingConstructionComponent.generated.h"

class AGridManager;
class AGridVisualizer;
class ABuildPreviewActor;
class ABuilding;

/**(Maxim)
 *
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UBuildingConstructionComponent : public UActorComponent
{
	GENERATED_BODY()

  public:
	UBuildingConstructionComponent();

	void Initialize( AGridManager* inGridManager, AGridVisualizer* inGridVisualizer );

	void StartPlacingBuilding( TSubclassOf<ABuilding> buildingClass );

	void CancelPlacing();

	void ConfirmPlacing();

	bool IsPlacing() const
	{
		return bIsPlacing_;
	}

  protected:
	void BeginPlay() override;

	void TickComponent( float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;

  private:
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager_ = nullptr;

	UPROPERTY()
	TObjectPtr<AGridVisualizer> GridVisualizer_ = nullptr;

	UPROPERTY()
	TObjectPtr<ABuildPreviewActor> PreviewActor_ = nullptr;

	UPROPERTY()
	TSubclassOf<ABuilding> CurrentBuildingClass_;

	bool bIsPlacing_ = false;

	FIntPoint CurrentCellCoords_ = FIntPoint( INDEX_NONE, INDEX_NONE );

	bool bHasValidCell_ = false;

	bool bCanBuildHere_ = false;

	void UpdateHoveredCell();

	bool FindCellUnderCursor( FIntPoint& outCellCoords, FVector& outCellWorldLocation ) const;

	bool CanBuildAtCell( const FIntPoint& cellCoords ) const;

	void UpdatePreviewVisual( const FVector& worldLocation, bool bCanBuild );

	void PlaceBuilding( const FIntPoint& cellCoords, const FVector& worldLocation );
};
