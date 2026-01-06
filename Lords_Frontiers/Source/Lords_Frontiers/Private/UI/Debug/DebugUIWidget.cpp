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

void UDebugUIWidget::OnButton9Clicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UResourceManager* ResManager = PC->FindComponentByClass<UResourceManager>();
	if (!ResManager)
	{
		ResManager = NewObject<UResourceManager>(PC, TEXT("DynamicResourceManager"));
		ResManager->RegisterComponent();
	}

	CreateResourceWidgets(ResManager);

	UEconomyComponent* EconomyComp = PC->FindComponentByClass<UEconomyComponent>();
	if (!EconomyComp)
	{
		EconomyComp = NewObject<UEconomyComponent>(PC, TEXT("DynamicEconomyComponent"));
		EconomyComp->RegisterComponent();
	}

	if (EconomyComp && ResManager)
	{
		EconomyComp->SetResourceManager(ResManager);

		UE_LOG(LogTemp, Warning, TEXT("UI Link: ResManager(%p) <-> Economy(%p)"), ResManager, EconomyComp);
	}

	if (EconomyComp)
	{
		EconomyComp->CollectGlobalResources();
	}
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

	if ( Button9 )
	{
		Button9->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton9Clicked );
		Button9->SetVisibility(ESlateVisibility::Visible);
	}

	if ( Button7 )
	{
		Button7->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton7Clicked );
		Button7->SetVisibility( ESlateVisibility::Visible );
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

UResourceManager* UDebugUIWidget::GetResourceManager() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	return PC ? PC->FindComponentByClass<UResourceManager>() : nullptr;
}

void UDebugUIWidget::OnResourceChangedHandler(EResourceType Type, int32 NewAmount)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta,
		FString::Printf(TEXT("UI RECEIVED: Type %d, Amount %d"), (uint8)Type, NewAmount));

	if (ResourceWidgetsMap.Contains(Type))
	{
		UResourceManager* ResManager = GetResourceManager();
		int32 MaxValue = ResManager ? ResManager->GetMaxResourceAmount(Type) : 100;

		ResourceWidgetsMap[Type]->UpdateAmount(NewAmount, MaxValue);
	}
}


void UDebugUIWidget::CreateResourceWidgets(UResourceManager* ResManager)
{

	if (ResourceWidgetsMap.Num() > 0) return;

	if (!ResManager || !ResourceContainer || !ResourceItemClass) return;

	ResManager->OnResourceChanged.AddDynamic(this, &UDebugUIWidget::OnResourceChangedHandler);

	for (uint8 i = 1; i < (uint8)EResourceType::MAX; ++i)
	{
		EResourceType CurrentType = static_cast<EResourceType>(i);
		UResourceItemWidget* NewItem = CreateWidget<UResourceItemWidget>(this, ResourceItemClass);
		if (NewItem)
		{
			TObjectPtr<UTexture2D> Icon = ResourceIcons.Contains(CurrentType) ? ResourceIcons[CurrentType] : nullptr;
			int32 StartValue = ResManager->GetResourceAmount(CurrentType);
			int32 MaxValue = ResManager->GetMaxResourceAmount(CurrentType);

			NewItem->SetupItem(Icon, StartValue, MaxValue);

			ResourceContainer->AddChild(NewItem);
			ResourceWidgetsMap.Add(CurrentType, NewItem);
		}
	}
}

