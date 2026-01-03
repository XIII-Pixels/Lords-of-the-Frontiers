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
#include "Lords_Frontiers/Public/ResourceManager/EconomyComponent.h"
#include "GameFramework/PlayerController.h"
#include "Lords_Frontiers/Public/ResourceManager/ResourceManager.h"

void UDebugUIWidget::OnButton1Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 1 Clicked" ) );
	}

	// Если ещё не знаем визуализатор — попробуем найти его сейчас.
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

	// Переключаем видимость доп. кнопок.
	bExtraButtonsVisible = !bExtraButtonsVisible;
	UpdateExtraButtonsVisibility();

	// Переключаем видимость сетки.
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

void UDebugUIWidget::OnButton9Clicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Button 9 Clicked (Collect Resources)"));
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	//get PlayerController
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("PlayerController not found"));
		return;
	}

	//warranty ResourceManager
	//EconomyComponent requires that the owner be ResourceManager
	UResourceManager* ResManager = PC->FindComponentByClass<UResourceManager>();
	if (!ResManager)
	{
		//if wallet not found - create him and attach to controller
		ResManager = NewObject<UResourceManager>(PC, TEXT("DynamicResourceManager"));
		if (ResManager)
		{
			ResManager->RegisterComponent(); //register component
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Created Dynamic ResourceManager"));
		}
	}

	CreateResourceWidgets();

	//warranty EconomyComponent
	UEconomyComponent* EconomyComp = PC->FindComponentByClass<UEconomyComponent>();
	if (!EconomyComp)
	{
		//if importer not found - create him
		EconomyComp = NewObject<UEconomyComponent>(PC, TEXT("DynamicEconomyComponent"));
		if (EconomyComp)
		{
			EconomyComp->RegisterComponent();
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Created Dynamic EconomyComponent"));
		}
	}

	//collection
	if (EconomyComp)
	{
		EconomyComp->CollectGlobalResources();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("CollectGlobalResources Called!"));
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Failed to create EconomyComponent!"));
	}
}

void UDebugUIWidget::OnButton7Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 7 Clicked (Relocate Building)" ) );
	}

	// 1) Убедимся, что у нас есть BuildManager
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

	// 3) Берём выделенное здание
	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: no building selected" ) );
		}
		return;
	}

	// 4) Запускаем перенос через BuildManager
	BuildManager->StartRelocatingBuilding( selectedBuilding );
}

bool UDebugUIWidget::Initialize()
{
	const ESlateVisibility newVis = bExtraButtonsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	const bool success = Super::Initialize();
	if ( !success )
	{
		return false;
	}

	// Привязка коллбеков кнопок.
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

	if ( Button9 )
	{
		Button9->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton9Clicked );
		Button9->SetVisibility(ESlateVisibility::Visible);
	}

	if ( Button7 )
	{
		Button7->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton7Clicked );
		Button7->SetVisibility( ESlateVisibility::Visible ); // по умолчанию скрыта
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
	if ( !Button7 || !SelectionManager )
	{
		return;
	}

	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	const bool bHasSelectedBuilding = ( selectedBuilding != nullptr );

	const ESlateVisibility newVis = bHasSelectedBuilding ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	Button7->SetVisibility( newVis );
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

void UDebugUIWidget::OnResourceChangedHandler(EResourceType Type, int32 NewAmount)
{
	if (ResourceWidgetsMap.Contains(Type))
	{
		UResourceManager* ResManager = UGameplayStatics::GetPlayerController(GetWorld(), 0)->FindComponentByClass<UResourceManager>();
		int32 MaxValue = ResManager ? ResManager->GetMaxResourceAmount(Type) : 100;

		ResourceWidgetsMap[Type]->UpdateAmount(NewAmount, MaxValue);
	}
}

void UDebugUIWidget::CreateResourceWidgets()
{

	if (ResourceWidgetsMap.Num() > 0) return;

	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	UResourceManager* ResManager = PC->FindComponentByClass<UResourceManager>();


	if (ResManager && ResourceContainer && ResourceItemClass)
	{
		ResManager->OnResourceChanged.AddDynamic(this, &UDebugUIWidget::OnResourceChangedHandler);

		for (uint8 i = 1; i <= (uint8)EResourceType::Progress; ++i)
		{
			EResourceType CurrentType = static_cast<EResourceType>(i);

			UResourceItemWidget* NewItem = CreateWidget<UResourceItemWidget>(this, ResourceItemClass);
			if (NewItem)
			{

				UTexture2D* Icon = ResourceIcons.Contains(CurrentType) ? ResourceIcons[CurrentType] : nullptr;

				int32 StartValue = ResManager->GetResourceAmount(CurrentType);
				int32 MaxValue = ResManager->GetMaxResourceAmount(CurrentType);

				NewItem->SetupItem(Icon, StartValue, MaxValue);
				NewItem->UpdateAmount(StartValue, MaxValue);

				ResourceContainer->AddChild(NewItem);
				ResourceWidgetsMap.Add(CurrentType, NewItem);
			}
		}
	}
}