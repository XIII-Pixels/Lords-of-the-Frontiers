#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CloudBorderManager.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;

/** Shape of the central cloud-free zone. */
UENUM()
enum class ECloudClearShape : uint8
{
	Circle,
	Square
};

USTRUCT()
struct FCloudInstanceData
{
	GENERATED_BODY()

	int32 ComponentIndex = 0;
	int32 InstanceIndex = 0;
	FTransform BaseTransform;
	FVector OutwardDirection;
};

UCLASS()
class LORDS_FRONTIERS_API ACloudBorderManager : public AActor
{
	GENERATED_BODY()

public:
	ACloudBorderManager();
	virtual void Tick( float DeltaTime ) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> CloudPlaneMesh;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TArray<TObjectPtr<UMaterialInterface>> CloudMaterials;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visuals" )
	float BaseCloudScale = 5.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Visuals" )
	FRotator CloudFacingRotation = FRotator( -50.0f, 45.0f, 0.0f );

	/** Half-size of the central clear zone that stays free of clouds (the playable area). */
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation",
	    meta = ( DisplayName = "Центральная свободная зона (полуразмер)" )
	)
	FVector2D MapExtents = FVector2D( 5000.0f, 5000.0f );

	/** Shape of the central clear zone: ellipse/circle or square. */
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation",
	    meta = ( DisplayName = "Форма свободной зоны" )
	)
	ECloudClearShape ClearZoneShape = ECloudClearShape::Square;

	/** Half-size of the whole area filled with clouds. Set >= half the view at max zoom-out (plus camera pan). */
	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation",
	    meta = ( DisplayName = "Зона покрытия облаками (полуразмер)" )
	)
	FVector2D CoverageExtents = FVector2D( 12000.0f, 12000.0f );

	/** Grid step between clouds. Smaller = denser (and many more instances). */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation" )
	float CloudSpacing = 800.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation" )
	float RandomOffsetRange = 300.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float MinZoomThreshold = 3000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float MaxZoomThreshold = 8000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float HideOffsetDown = 1500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float HideOffsetOutward = 2000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Animation" )
	float TransitionSpeed = 5.0f;

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CloudISMCs;

	TArray<FCloudInstanceData> CloudsData;

	void GenerateClouds();
	bool IsInsideClearZone( float x, float y ) const;
	float CurrentZoomAlpha = 0.0f;
};