// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/LevelButton.h"

#include "Components/Button.h"

void ULevelButton::NativeConstruct()
{
	Super::NativeConstruct();

	Butt->OnClicked.AddDynamic( this, &ULevelButton::HandleClick );
}

void ULevelButton::HandleClick()
{
	OnClicked.Broadcast( LevelIndex );
}
