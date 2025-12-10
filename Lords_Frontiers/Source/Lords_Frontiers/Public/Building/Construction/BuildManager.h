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

/// Менеджер строительства: выбор клетки, превью и спавн здания.
UCLASS()
class LORDS_FRONTIERS_API ABuildManager : public AActor
{
	GENERATED_BODY()

  public:
	ABuildManager();

	virtual void Tick( float deltaSeconds ) override;

	/// @brief Начать перенос уже существующего здания.
	UFUNCTION( BlueprintCallable, Category = "Building" )
	void StartRelocatingBuilding( ABuilding* buildingToMove );

	/// @brief Сейчас идёт перенос здания (а не обычное строительство)?
	UFUNCTION( BlueprintPure, Category = "Building" )
	bool IsRelocating() const
	{
		return bIsRelocating_;
	}

	/// Начать режим строительства указанного класса здания.
	UFUNCTION( BlueprintCallable, Category = "Building" )
	void StartPlacingBuilding( TSubclassOf<ABuilding> buildingClass );

	/// <summary>
	/// </summary>
	/// <param name="cellCoords"></param>
	/// <returns>Вверх справа низу слева</returns>
	TArray<TWeakObjectPtr<ABuilding>> GetBuildingsNearby( FIntPoint cellCoords );

	UFUNCTION( BlueprintCallable, Category = "Building" )
	void CancelPlacing();

	/// Подтвердить строительство (если возможно) и заспавнить здание.
	UFUNCTION( BlueprintCallable, Category = "Building" )
	void ConfirmPlacing();

	/// Активен ли сейчас режим строительства.
	UFUNCTION( BlueprintPure, Category = "Building" )
	bool IsPlacing() const
	{
		return bIsPlacing_;
	}

  protected:
	virtual void BeginPlay() override;

	/// Логический менеджер сетки.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridManager> GridManager_ = nullptr;

	/// Визуализатор сетки.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TObjectPtr<AGridVisualizer> GridVisualizer_ = nullptr;

	/// Класс превью-актора (можно переопределить в BP).
	UPROPERTY( EditAnywhere, Category = "Settings|Preview", meta = ( AllowPrivateAccess = "true" ) )
	TSubclassOf<ABuildPreviewActor> PreviewActorClass_;

	/// Класс превью-актора для стены (можно переопределить в BP).
	UPROPERTY( EditAnywhere, Category = "Settings|Preview", meta = ( AllowPrivateAccess = "true" ) )
	TSubclassOf<ABuildPreviewActor> PreviewActorWall_;

  private:
	/// Текущий превью-актор (призрак здания).
	UPROPERTY()
	TObjectPtr<ABuildPreviewActor> PreviewActor_ = nullptr;

	/// Класс здания, выбранный для строительства.
	UPROPERTY()
	TSubclassOf<ABuilding> CurrentBuildingClass_;

	// Актора-превью для сегментов стен (максимум 4 - по сторонам света).
	UPROPERTY()
	TArray<TObjectPtr<ABuildPreviewActor>> WallSegmentPreviewActors_;

	UPROPERTY()
	TObjectPtr<ABuilding> RelocatedBuilding_ = nullptr;

	/// @brief Координаты клетки, где здание стояло до начала переноса.
	FIntPoint OriginalCellCoords_ = FIntPoint( -1, -1 );

	/// @brief Флаг режима переноса (true — перенос существующего здания).
	bool bIsRelocating_ = false;

	// Обновить превью стен вокруг текущей клетки.
	// void UpdateWallPreviews( const FIntPoint& cellCoords, const FVector& cellWorldLocation );

	// Скрыть все превью сегментов стены.
	void HideAllWallPreviews();
	/// Активен ли режим строительства.
	UPROPERTY()
	bool bIsPlacing_ = false;

	/// Есть ли валидная клетка под курсором.
	UPROPERTY()
	bool bHasValidCell_ = false;

	/// Можно ли строить на текущей клетке.
	UPROPERTY()
	bool bCanBuildHere_ = false;

	/// Текущие координаты клетки под курсором.
	UPROPERTY()
	FIntPoint CurrentCellCoords_ = FIntPoint::ZeroValue;

	/// Обновить клетку под курсором и превью.
	void UpdateHoveredCell();

	/// Обновить позицию и цвет превью.
	void UpdatePreviewVisual( const FVector& worldLocation, bool bCanBuild );
};
