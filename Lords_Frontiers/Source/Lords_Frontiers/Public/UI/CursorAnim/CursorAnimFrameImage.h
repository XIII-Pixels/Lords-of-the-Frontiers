#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"
#include "UI/Widgets/AlphaHitImage.h"

#include "CursorAnimFrameImage.generated.h"

class FReply;
struct FGeometry;
struct FPointerEvent;

UCLASS()
class LORDS_FRONTIERS_API UCursorAnimFrameImage : public UAlphaHitImage
{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CursorAnim" )
	FGameplayTag AnimationTag;

protected:
	virtual void SynchronizeProperties() override;

private:
	FReply HandleMouseButtonDown( const FGeometry& geometry, const FPointerEvent& mouseEvent );
};
