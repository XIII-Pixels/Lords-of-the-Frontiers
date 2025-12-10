// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Selectable.h"

#include "SelectionManagerComponent.generated.h"

class ABuilding;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSelectionChanged );
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API USelectionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

  public:
	USelectionManagerComponent();

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	void SelectSingle( AActor* actor );

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	void ClearSelection();

	UFUNCTION( BlueprintPure, Category = "Settings|Selection" )
	AActor* GetPrimarySelectedActor() const;

	UFUNCTION( BlueprintPure, Category = "Settings|Selection" )
	ABuilding* GetPrimarySelectedBuilding() const;

	UPROPERTY( BlueprintAssignable, Category = "Settings|Selection" )
	FOnSelectionChanged OnSelectionChanged;

  protected:
	virtual void BeginPlay() override;

  private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SelectedActors_;
};
