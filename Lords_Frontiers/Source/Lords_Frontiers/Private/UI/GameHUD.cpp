#include "Lords_Frontiers/Public/UI/GameHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Components/VerticalBox.h"
#include "Components/GridPanel.h"



void UGameHUDWidget::NativeConstruct ()
{
    Super::NativeConstruct ();

    if ( ButtonRelocateBuilding )
    {
        ButtonRelocateBuilding->OnClicked.AddDynamic ( this, &UGameHUDWidget::OnRelocateBuildingClicked );
    }
    if ( ButtonRemoveBuilding )
    {
        ButtonRemoveBuilding->OnClicked.AddDynamic ( this, &UGameHUDWidget::OnRemoveBuildingClicked );
    }
    if ( ButtonDefensiveBuildings )
    {
        ButtonDefensiveBuildings->OnClicked.AddDynamic ( this, &UGameHUDWidget::OnDefensiveBuildingsClicked );
    }
    if ( ButtonEconomyBuilding )
    {
        ButtonEconomyBuilding->OnClicked.AddDynamic ( this, &UGameHUDWidget::OnEconomyBuildingClicked );
    }
    if ( ButtonEndTurn )
    {
        ButtonEndTurn->OnClicked.AddDynamic ( this, &UGameHUDWidget::OnEndTurnClicked );
    }

    ShowEconomyBuildings ();
}

void UGameHUDWidget::NativeDestruct ()
{
    // Unbind
    if ( ButtonRelocateBuilding ) ButtonRelocateBuilding->OnClicked.RemoveDynamic ( this, &UGameHUDWidget::OnRelocateBuildingClicked );
    if ( ButtonRemoveBuilding ) ButtonRemoveBuilding->OnClicked.RemoveDynamic ( this, &UGameHUDWidget::OnRemoveBuildingClicked );
    if ( ButtonDefensiveBuildings ) ButtonDefensiveBuildings->OnClicked.RemoveDynamic ( this, &UGameHUDWidget::OnDefensiveBuildingsClicked );
    if ( ButtonEconomyBuilding ) ButtonEconomyBuilding->OnClicked.RemoveDynamic ( this, &UGameHUDWidget::OnEconomyBuildingClicked );
    if ( ButtonEndTurn ) ButtonEndTurn->OnClicked.RemoveDynamic ( this, &UGameHUDWidget::OnEndTurnClicked );

    Super::NativeDestruct ();
}

void UGameHUDWidget::OnRelocateBuildingClicked ()
{
    UE_LOG ( LogTemp, Log, TEXT ( "Relocate building clicked" ) );
}

void UGameHUDWidget::OnRemoveBuildingClicked ()
{
    UE_LOG ( LogTemp, Log, TEXT ( "Remove building clicked" ) );
}

void UGameHUDWidget::OnDefensiveBuildingsClicked ()
{
    ShowDefensiveBuildings ();
}

void UGameHUDWidget::OnEconomyBuildingClicked ()
{
    ShowEconomyBuildings ();
}

void UGameHUDWidget::OnEndTurnClicked ()
{
    UE_LOG ( LogTemp, Log, TEXT ( "End Turn clicked" ) );
}

void UGameHUDWidget::ShowEconomyBuildings ()
{
    if ( EconomyCardBox )
    {
        EconomyCardBox->SetVisibility ( ESlateVisibility::Visible );
    }

    if ( DefensiveCardBox )
    {
        DefensiveCardBox->SetVisibility ( ESlateVisibility::Collapsed );
    }
}

void UGameHUDWidget::ShowDefensiveBuildings ()
{
    if ( EconomyCardBox )
    {
        EconomyCardBox->SetVisibility ( ESlateVisibility::Collapsed );
    }

    if ( DefensiveCardBox )
    {
        DefensiveCardBox->SetVisibility ( ESlateVisibility::Visible );
    }
}