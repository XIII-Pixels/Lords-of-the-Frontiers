// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/UIManager.h"

#include "Blueprint/UserWidget.h"

void UUIManager::OnStartPlay()
{
	if ( Widget_ )
	{
		Widget_->AddToViewport();
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "UUIManager: failed to show widget. Widget_ is nullptr" ) );
	}
}
