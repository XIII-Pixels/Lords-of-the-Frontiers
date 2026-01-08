// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Debug/DebugUIWidget.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Components/Button.h"
#include "Core/Debug/DebugPlayerController.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "Engine/Engine.h"
#include "Grid/GridVisualizer.h"
#include "Kismet/GameplayStatics.h"

void UDebugUIWidget::OnButton1Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 1 Clicked" ) );
	}

	if ( !GridVisualizer )
	{
		if ( UWorld* world = GetWorld() )
		{
			GridVisualizer =
			    Cast<AGridVisualizer>( UGameplayStatics::GetActorOfClass( world, AGridVisualizer::StaticClass() ) );
		}

		if ( !GridVisualizer )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "GridVisualizer not found" ) );
			}
			return;
		}
	}

	FLinearColor CurrentColor = Button1->GetBackgroundColor();
	if ( CurrentColor.Equals( FLinearColor::Red, 0.01f ) )
	{
		Button1->SetBackgroundColor( FLinearColor::Green );
	}
	else
	{
		Button1->SetBackgroundColor( FLinearColor::Red );
	}

	bExtraButtonsVisible = !bExtraButtonsVisible;
	UpdateExtraButtonsVisibility();

	if ( GridVisualizer->IsGridVisible() )
	{
		GridVisualizer->HideGrid();
	}
	else
	{
		GridVisualizer->ShowGrid();
	}
}

void UDebugUIWidget::OnButton2Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 2 Clicked (Build)" ) );
	}

	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "BuildManager not found" ) );
		}
		return;
	}

	if ( !Button2BuildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Button2BuildingClass is not set" ) );
		}
		return;
	}

	BuildManager->StartPlacingBuilding( Button2BuildingClass );
}

void UDebugUIWidget::OnButton3Clicked()
{
	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "BuildManager not found" ) );
		}
		return;
	}

	if ( !Button3BuildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Button3BuildingClass is not set" ) );
		}
		return;
	}

	BuildManager->StartPlacingBuilding( Button3BuildingClass );
}

void UDebugUIWidget::OnButton4Clicked()
{
	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "BuildManager not found" ) );
		}
		return;
	}

	if ( !Button4BuildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Button4BuildingClass is not set" ) );
		}
		return;
	}

	BuildManager->StartPlacingBuilding( Button4BuildingClass );
}

void UDebugUIWidget::OnButton7Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 7 Clicked (Relocate Building)" ) );
	}

	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: BuildManager not found" )
			);
		}
		return;
	}

	if ( !SelectionManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: SelectionManager not found" )
			);
		}
		return;
	}

	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: no building selected" ) );
		}
		return;
	}

	BuildManager->StartRelocatingBuilding( selectedBuilding );
}

void UDebugUIWidget::OnButton8Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT( "Button 8 Clicked (Relocate Building)" ) );
	}

	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT( "OnButton8Clicked: no building selected" ) );
		}
		return;
	}
	BuildManager->DemolitionsPlacing( selectedBuilding );
	SelectionManager->ClearSelection();
	HandleSelectionChanged();
	UpdateExtraButtonsVisibility();
}

bool UDebugUIWidget::Initialize()
{
	const ESlateVisibility newVis = bExtraButtonsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	const bool success = Super::Initialize();
	if ( !success )
	{
		return false;
	}

	if ( Button1 )
	{
		Button1->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton1Clicked );
	}
	if ( Button2 )
	{
		Button2->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton2Clicked );
		Button2->SetVisibility( newVis );
	}
	if ( Button3 )
	{
		Button3->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton3Clicked );
		Button3->SetVisibility( newVis );
	}
	if ( Button4 )
	{
		Button4->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton4Clicked );
	}

	if ( Button7 )
	{
		Button7->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton7Clicked );
		Button7->SetVisibility( ESlateVisibility::Visible );
	}
	if ( Button8 )
	{
		Button8->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton8Clicked );
		Button8->SetVisibility( ESlateVisibility::Visible );
	}
	if ( UWorld* world = GetWorld() )
	{
		if ( ADebugPlayerController* debugPC =
		         Cast<ADebugPlayerController>( UGameplayStatics::GetPlayerController( world, 0 ) ) )
		{
			SelectionManager = debugPC->GetSelectionManager();

			if ( SelectionManager )
			{
				SelectionManager->OnSelectionChanged.AddDynamic( this, &UDebugUIWidget::HandleSelectionChanged );

				HandleSelectionChanged();
			}
		}
	}

	if ( !SelectionManager && GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
		    -1, 2.0f, FColor::Red, TEXT( "UDebugUIWidget: SelectionManager is NULL in Initialize" )
		);
	}
	return true;
}

void UDebugUIWidget::HandleSelectionChanged()
{
	if ( !Button7 || !Button8 || !SelectionManager )
	{
		return;
	}

	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	const bool bHasSelectedBuilding = ( selectedBuilding != nullptr );

	const ESlateVisibility newVis = bHasSelectedBuilding ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	Button7->SetVisibility( newVis );
	Button8->SetVisibility( newVis );
}

void UDebugUIWidget::UpdateExtraButtonsVisibility()
{
	const ESlateVisibility newVis = bExtraButtonsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if ( Button2 )
	{
		Button2->SetVisibility( newVis );
	}
	if ( Button3 )
	{
		Button3->SetVisibility( newVis );
	}
}

void UDebugUIWidget::InitSelectionManager( USelectionManagerComponent* InSelectionManager )
{
	SelectionManager = InSelectionManager;

	if ( !SelectionManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "UDebugUIWidget::InitSelectionManager: InSelectionManager is null" )
			);
		}
		return;
	}

	SelectionManager->OnSelectionChanged.AddDynamic( this, &UDebugUIWidget::HandleSelectionChanged );

	HandleSelectionChanged();
}