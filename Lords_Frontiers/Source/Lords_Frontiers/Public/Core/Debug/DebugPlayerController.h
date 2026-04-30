// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "DebugPlayerController.generated.h"

class USelectionManagerComponent;
class ABuildManager;
class UCardSelectionHUDComponent;

/**
 *
 */
UCLASS()
class LORDS_FRONTIERS_API ADebugPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADebugPlayerController();

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	USelectionManagerComponent* GetSelectionManager() const;

	UFUNCTION( Exec )
	void Card_ToggleDebug();

	UFUNCTION( Exec )
	void Card_Apply( FName cardID );

	UFUNCTION( Exec )
	void Card_Unlock( FName cardID );

	UFUNCTION( Exec )
	void Card_Lock( FName cardID );

	UFUNCTION( Exec )
	void Card_ResetUnlocks();

protected:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Selection|Components" )
	TObjectPtr<USelectionManagerComponent> SelectionManagerComponent_ = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Cards" )
	TObjectPtr<UCardSelectionHUDComponent> CardSelectionHUDComponent_ = nullptr;

	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<ABuildManager> BuildManager_ = nullptr;

	void HandleLeftClick();
	void HandleRightClick();
	void HandleEscape();

	void EnsureBuildManager();
};
