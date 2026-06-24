#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "CursorAnimationSubsystem.generated.h"

class UCursorAnimationConfig;
class UCursorAnimationPlayerWidget;

UCLASS()
class LORDS_FRONTIERS_API UCursorAnimationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCursorAnimationSubsystem* Get( const UObject* worldContextObject );

	virtual void Deinitialize() override;

	UFUNCTION( BlueprintCallable, Category = "CursorAnim" )
	void SetConfig( UCursorAnimationConfig* config );

	void PlayAtScreenPosition( const FGameplayTag& tag, const FVector2D& screenPosition );

private:
	UCursorAnimationPlayerWidget* AcquirePlayer();
	void HandlePlayerFinished( UCursorAnimationPlayerWidget* player );

	UPROPERTY()
	TObjectPtr<UCursorAnimationConfig> Config_;

	UPROPERTY()
	TArray<TObjectPtr<UCursorAnimationPlayerWidget>> FreePool_;

	UPROPERTY()
	TSet<TObjectPtr<UCursorAnimationPlayerWidget>> InUse_;
};
