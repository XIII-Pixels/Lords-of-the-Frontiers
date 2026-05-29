#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"

#include "CursorAnimationConfig.generated.h"

class UCursorAnimationPlayerWidget;
class UTexture2D;

USTRUCT( BlueprintType )
struct FCursorAnimationEntry
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "CursorAnim" )
	FGameplayTag Tag;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "CursorAnim" )
	TArray<TObjectPtr<UTexture2D>> Frames;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "CursorAnim", meta = ( ClampMin = "0.01" ) )
	float FramesPerSecond = 15.0f;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "CursorAnim" )
	FVector2D DisplaySize = FVector2D( 128.0f, 128.0f );
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCursorAnimationConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	const FCursorAnimationEntry* FindByTag( const FGameplayTag& tag ) const;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<UCursorAnimationPlayerWidget> PlayerWidgetClass;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	int32 ViewportZOrder = 1000;

private:

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TArray<FCursorAnimationEntry> Animations_;
};
