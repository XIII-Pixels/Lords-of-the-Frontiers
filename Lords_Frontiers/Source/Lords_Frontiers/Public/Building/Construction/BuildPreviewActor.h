// BuildPreviewActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BuildPreviewActor.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/**(Maxim)
 *
 */
UCLASS()
class LORDS_FRONTIERS_API ABuildPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildPreviewActor();

	UFUNCTION( BlueprintCallable, Category = "Settings|Preview" )
	void SetCanBuild( bool bCanBuild );

	UFUNCTION( BlueprintCallable, Category = "Settings|Preview" )
	void SetPreviewMesh( UStaticMesh* mesh );

protected:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UStaticMeshComponent> MeshComponent_ = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Preview" )
	TObjectPtr<UMaterialInterface> ValidMaterial_ = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Preview" )
	TObjectPtr<UMaterialInterface> InvalidMaterial_ = nullptr;
};
