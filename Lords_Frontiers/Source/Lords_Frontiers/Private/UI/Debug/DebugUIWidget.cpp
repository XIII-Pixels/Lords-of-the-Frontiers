// Fill out your copyright notice in the Description page of Project Settings.


#include "Lords_Frontiers/Public/UI/Debug/DebugUIWidget.h"


void UDebugUIWidget::OnButton1Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Button 1 Clicked");
}

void UDebugUIWidget::OnButton2Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Button 2 Clicked");
}

void UDebugUIWidget::OnButton3Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Button 3 Clicked");
}

void UDebugUIWidget::OnButton4Clicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Button 4 Clicked");
}

bool UDebugUIWidget::Initialize()
{
	bool success = Super::Initialize();
	if ( !success )
	{
		return false;
	}

	// (Gregory-hub)
	// Set callbacks for buttons
	if ( Button1 )
	{
		Button1->OnClicked.AddDynamic(this, &UDebugUIWidget::OnButton1Clicked);
	}
	if ( Button2 )
	{
		Button2->OnClicked.AddDynamic(this, &UDebugUIWidget::OnButton2Clicked);
	}
	if ( Button3 )
	{
		Button3->OnClicked.AddDynamic(this, &UDebugUIWidget::OnButton3Clicked);
	}
	if ( Button4 )
	{
		Button4->OnClicked.AddDynamic(this, &UDebugUIWidget::OnButton4Clicked);
	}
	
	return true;
}
