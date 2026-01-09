#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GridVisualizer.generated.h"

class AGridManager;
class UInstancedStaticMeshComponent;

/**(Maxim)
 *
 */
UCLASS()
class LORDS_FRONTIERS_API AGridVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AGridVisualizer();

	UFUNCTION( BlueprintCallable, Category = "Settings|Grid" )
	void ShowGrid();

	UFUNCTION( BlueprintCallable, Category = "Settings|Grid" )
	void HideGrid();

	UFUNCTION( BlueprintCallable, Category = "Settings|Grid" )
	void SetGridVisible( bool bVisible );

	UFUNCTION( BlueprintCallable, Category = "Settings|Grid" )
	bool IsGridVisible() const
	{
		return bGridVisible_;
	}

	UFUNCTION( BlueprintCallable, Category = "Settings|Grid" )
	bool GetCellWorldCenter( const FIntPoint& CellCoords, FVector& OutLocation ) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridManager> GridManager_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	bool bGridVisible_ = false;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	float ZOffset_ = 5.0f;

	UPROPERTY( VisibleAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<UInstancedStaticMeshComponent> BuildableMesh_;

	UPROPERTY( VisibleAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<UInstancedStaticMeshComponent> BlockedMesh_;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	float MeshScale_ = 0.48f;

private:
	void RefreshGrid();
};
