// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Debug/DebugPathUIWidget.h"

#include "Waves/WaveManager.h"

#include "EngineUtils.h"

void UDebugPathUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Button1 )
	{
		Button1->OnClicked.RemoveAll( this );
		Button1->OnClicked.AddDynamic( this, &UDebugPathUIWidget::OnButton1Clicked );
	}

	if ( Button2 )
	{
		Button2->OnClicked.RemoveAll( this );
		Button2->OnClicked.AddDynamic( this, &UDebugPathUIWidget::OnButton2Clicked );
	}

	for (TActorIterator<AWaveManager> it(GetWorld()); it; ++it)
	{
		WaveManager_ = *it;
		break;
	}
}

void UDebugPathUIWidget::OnButton1Clicked()
{
	if ( !WaveManager_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "WaveManager_.IsValid() == false" ) );
		return;
	}

	WaveManager_->StartWaves();
}

void UDebugPathUIWidget::OnButton2Clicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Button 2 clicked" ) );
}
