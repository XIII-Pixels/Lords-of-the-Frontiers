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

/// �������� �������������: ����� ������, ������ � ����� ������.
UCLASS()
class LORDS_FRONTIERS_API ABuildManager : public AActor
{
	GENERATED_BODY()

public:
	ABuildManager();

	virtual void Tick( float deltaSeconds ) override;

	/// @brief ������ ������� ��� ������������� ������.
	UFUNCTION( BlueprintCallable, Category = "Building" )
	void StartRelocatingBuilding( ABuilding* buildingToMove );

	/// @brief ������ ��� ������� ������ (� �� ������� �������������)?
	UFUNCTION( BlueprintPure, Category = "Building" )
	bool IsRelocating() const
	{
		return bIsRelocating_;
	}

	/// ������ ����� ������������� ���������� ������ ������.
	UFUNCTION( BlueprintCallable, Category = "Building" )
	void StartPlacingBuilding( TSubclassOf<ABuilding> buildingClass );

	/// <summary>
	/// </summary>
	/// <param name="cellCoords"></param>
	/// <returns>����� ������ ���� �����</returns>
	TArray<TWeakObjectPtr<ABuilding>> GetBuildingsNearby( FIntPoint cellCoords );

	UFUNCTION( BlueprintCallable, Category = "Building" )
	void CancelPlacing();

	/// ����������� ������������� (���� ��������) � ���������� ������.
	UFUNCTION( BlueprintCallable, Category = "Building" )
	void ConfirmPlacing();

	/// ������� �� ������ ����� �������������.
	UFUNCTION( BlueprintPure, Category = "Building" )
	bool IsPlacing() const
	{
		return bIsPlacing_;
	}

protected:
	virtual void BeginPlay() override;

	/// ���������� �������� �����.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridManager> GridManager_ = nullptr;

	/// ������������ �����.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridVisualizer> GridVisualizer_ = nullptr;

	/// ����� ������-������ (����� �������������� � BP).
	UPROPERTY( EditAnywhere, Category = "Settings|Preview", meta = ( AllowPrivateAccess = "true" ) )
	TSubclassOf<ABuildPreviewActor> PreviewActorClass_;

	/// ����� ������-������ ��� ����� (����� �������������� � BP).
	UPROPERTY( EditAnywhere, Category = "Settings|Preview", meta = ( AllowPrivateAccess = "true" ) )
	TSubclassOf<ABuildPreviewActor> PreviewActorWall_;

private:
	/// ������� ������-����� (������� ������).
	UPROPERTY()
	TObjectPtr<ABuildPreviewActor> PreviewActor_ = nullptr;

	/// ����� ������, ��������� ��� �������������.
	UPROPERTY()
	TSubclassOf<ABuilding> CurrentBuildingClass_;

	// ������-������ ��� ��������� ���� (�������� 4 - �� �������� �����).
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

	// ������ ��� ������ ��������� �����.
	void HideAllWallPreviews();
	/// ������� �� ����� �������������.
	UPROPERTY()
	bool bIsPlacing_ = false;

	/// ���� �� �������� ������ ��� ��������.
	UPROPERTY()
	bool bHasValidCell_ = false;

	/// ����� �� ������� �� ������� ������.
	UPROPERTY()
	bool bCanBuildHere_ = false;

	/// ������� ���������� ������ ��� ��������.
	UPROPERTY()
	FIntPoint CurrentCellCoords_ = FIntPoint::ZeroValue;

	/// �������� ������ ��� �������� � ������.
	void UpdateHoveredCell();

	/// �������� ������� � ���� ������.
	void UpdatePreviewVisual( const FVector& worldLocation, bool bCanBuild );
};
