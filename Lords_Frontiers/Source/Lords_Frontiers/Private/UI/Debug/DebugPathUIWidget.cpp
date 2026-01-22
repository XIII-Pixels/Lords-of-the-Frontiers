// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Debug/DebugPathUIWidget.h"

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

	Path_ = NewObject<UPath>(this);
	Path_->AutoSetGrid();
	Path_->SetStartAndGoal( { 0, 0 }, { 5, 5 } );
}

void UDebugPathUIWidget::OnButton1Clicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Button 1 clicked" ) );
	Path_->CalculateOrUpdate();
}

void UDebugPathUIWidget::OnButton2Clicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Button 2 clicked" ) );
}
