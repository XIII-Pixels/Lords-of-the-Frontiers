// BuildManager.h

#pragma once

#include "Building/Construction/BuildingPlacementUtils.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BuildManager.generated.h"

class AGridManager;
class AGridVisualizer;
class ABuildPreviewActor;
class ABuilding;
class UResourceManager;
class UBonusIconsData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnBonusPreviewUpdated, const TArray<FBonusIconData>&, BonusIcons );

UCLASS()
class LORDS_FRONTIERS_API ABuildManager : public AActor
{
	GENERATED_BODY()

public:
	ABuildManager();

	virtual void Tick( float deltaSeconds ) override;

	UFUNCTION( BlueprintCallable, Category = "Settings|Building" )
	void StartRelocatingBuilding( ABuilding* buildingToMove );

	UFUNCTION( BlueprintPure, Category = "Settings|Building" )
	bool IsRelocating() const
	{
		return bIsRelocating_;
	}

	UFUNCTION( BlueprintCallable, Category = "Settings|Building" )
	void StartPlacingBuilding( TSubclassOf<ABuilding> buildingClass );

	/// <summary>
	/// </summary>
	/// <param name="cellCoords"></param>
	/// <returns></returns>
	TArray<TWeakObjectPtr<ABuilding>> GetBuildingsNearby( FIntPoint cellCoords );

	UFUNCTION( BlueprintCallable, Category = "Settings|Building" )
	void CancelPlacing();

	void ResetPlacementState();

	void RelocateExistingBuilding( const FVector& cellWorldLocation );

	bool TryPlaceNewBuilding( const FVector& cellWorldLocation );

	bool ValidatePlacement( FVector& outCellWorldLocation ) const;

	UFUNCTION( BlueprintCallable, Category = "Settings|Building" )
	void ConfirmPlacing();

	UFUNCTION( BlueprintPure, Category = "Settings|Building" )
	bool IsPlacing() const
	{
		return bIsPlacing_;
	};

	UPROPERTY( BlueprintAssignable, Category = "Settings|Bonus" )
	FOnBonusPreviewUpdated OnBonusPreviewUpdated;

	void RecalculateBonusesAroundBuilding( ABuilding* building, const FIntPoint& cellCoords );

	void RecalculateBonusesFromNeighbors( const int32 MaxBonusRadius, const FIntPoint& cellCoords );

	TArray<FBonusIconData> CollectBonusPreview( TSubclassOf<ABuilding> buildingClass, const FIntPoint& cellCoords );

	void CollectBonusFromNeighbors( const FIntPoint& myCellCoords, TArray<FBonusIconData>& result );

	void DebugMessage( const FColor& color, const FString& message, float duration = 1.5f );

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TObjectPtr<UBonusIconsData> BonusIconsData;

	void ShowBonusHighlightForBuilding( TSubclassOf<ABuilding> buildingClass );

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridManager> GridManager_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridVisualizer> GridVisualizer_;

	UPROPERTY( EditAnywhere, Category = "Settings|Preview", meta = ( AllowPrivateAccess = "true" ) )
	TSubclassOf<ABuildPreviewActor> PreviewActorClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Preview", meta = ( AllowPrivateAccess = "true" ) )
	TSubclassOf<ABuildPreviewActor> PreviewActorWall_;

	void
	CollectBonusFromNeighbors( const FIntPoint& myCellCoords, TArray<FBonusIconData>& result, const ABuilding* cdo );

private:
	UPROPERTY()
	TWeakObjectPtr<UResourceManager> CachedResourceManager_;

	UFUNCTION()
	void OnCoreSystemsReady();

	FIntPoint PreviousCellCoords_ = FIntPoint( INDEX_NONE, INDEX_NONE );

	TArray<FBonusIconData> CachedBonusIcons_;

	void AppendMatchingBonuses(
	    UBuildingBonusComponent* BonusComp, TSubclassOf<ABuilding> MatchClass, const FVector& IconWorldLocation,
	    TArray<FBonusIconData>& OutResult
	);

	TArray<FBonusIconData> AggregateBonusIcons( const TArray<FBonusIconData>& rawIcons );

	UPROPERTY()
	TObjectPtr<ABuildPreviewActor> PreviewActor_ = nullptr;

	UPROPERTY()
	TSubclassOf<ABuilding> CurrentBuildingClass_;

	UPROPERTY()
	TArray<TObjectPtr<ABuildPreviewActor>> WallSegmentPreviewActors_;

	UPROPERTY()
	TObjectPtr<ABuilding> RelocatedBuilding_ = nullptr;

	/// @brief ���������� ������, ��� ������ ������ �� ������ ��������.
	FIntPoint OriginalCellCoords_ = FIntPoint( -1, -1 );

	/// @brief ���� ������ �������� (true � ������� ������������� ������).
	bool bIsRelocating_ = false;

	// �������� ������ ���� ������ ������� ������.
	// void UpdateWallPreviews( const FIntPoint& cellCoords, const FVector&
	// cellWorldLocation );

	void HideAllWallPreviews();

	UPROPERTY()
	bool bIsPlacing_ = false;

	UPROPERTY()
	bool bHasValidCell_ = false;

	UPROPERTY()
	bool bCanBuildHere_ = false;

	UPROPERTY()
	FIntPoint CurrentCellCoords_ = FIntPoint::ZeroValue;

	void UpdateHoveredCell();

	void UpdatePreviewVisual( const FVector& worldLocation, bool bCanBuild );
};
