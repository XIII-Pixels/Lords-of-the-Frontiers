#pragma once

#include "Cards/CardTypes.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CardSelectionHUDComponent.generated.h"

class UCardSelectionWidget;
class UCardSubsystem;
class UCardDataAsset;

/**
 * UCardSelectionHUDComponent
 *
 * Automatically shows card selection UI when CardSubsystem requests it.
 * Add to PlayerController.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UCardSelectionHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCardSelectionHUDComponent();

	UFUNCTION( BlueprintCallable, Category = "Card Selection" )
	void ShowCardSelection( const FCardChoice& choice );

	UFUNCTION( BlueprintCallable, Category = "Card Selection" )
	void HideCardSelection();

	UFUNCTION( BlueprintPure, Category = "Card Selection" )
	bool IsCardSelectionVisible() const;

	UFUNCTION( BlueprintPure, Category = "Card Selection" )
	UCardSelectionWidget* GetCardSelectionWidget() const
	{
		return CardSelectionWidget_;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Card Selection" )
	TSubclassOf<UCardSelectionWidget> CardSelectionWidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Card Selection" )
	bool bAutoSubscribe = true;

private:
	UPROPERTY()
	TObjectPtr<UCardSelectionWidget> CardSelectionWidget_;

	TWeakObjectPtr<UCardSubsystem> CachedCardSubsystem_;

	bool bIsSubscribed_ = false;

	void SubscribeToCardSubsystem();
	void UnsubscribeFromCardSubsystem();

	UFUNCTION()
	void HandleCardSelectionRequired( const FCardChoice& choice );

	UFUNCTION()
	void HandleSelectionComplete( const TArray<UCardDataAsset*>& selectedCards );

	UCardSubsystem* GetCardSubsystem();
	void EnsureWidgetCreated();
};
