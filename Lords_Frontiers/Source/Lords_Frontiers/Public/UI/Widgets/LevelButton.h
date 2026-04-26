// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core/Saving/GameSaveData.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "LevelButton.generated.h"

class UImage;
enum class ELevelStatus;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLevelClicked, int32, LevelIndex );

/** (Gregory-hub)
 * Represents button in level choosing menu */
UCLASS( Abstract )
class LORDS_FRONTIERS_API ULevelButton : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClick();

	void SetStateLocked();
	void SetStateUnlocked();
	void SetStateCompleted();

	int LevelIndex() const
	{
		return LevelIndex_;
	}

	UPROPERTY( BlueprintAssignable )
	FOnLevelClicked OnClicked;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> Butt;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UImage> StatusImage;

protected:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	int LevelIndex_ = -1;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	TObjectPtr<UTexture2D> TextureLocked_ = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	TObjectPtr<UTexture2D> TextureUnlocked_ = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	TObjectPtr<UTexture2D> TextureCompleted_ = nullptr;
};
