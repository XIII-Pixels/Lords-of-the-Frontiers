// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LevelChoosingMenu.h"

#include "Localization/GameLocalization.h"
#include "UI/Widgets/TextButtonWidget.h"

#include "Components/TextBlock.h"

void ULevelChoosingMenu::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLocalizedLabels();
}

void ULevelChoosingMenu::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyLocalizedLabels();
}

void ULevelChoosingMenu::ApplyLocalizedLabels()
{
	if ( TitleText )
	{
		TitleText->SetText( FText::FromStringTable( LordsFrontiersLoc::GetTableId(), TEXT( "LevelSelect.Title" ) ) );
	}

	if ( BackButton )
	{
		BackButton->SetLabelKey( FName( TEXT( "LevelSelect.Back" ) ) );
	}
}
