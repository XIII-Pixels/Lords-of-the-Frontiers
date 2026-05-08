#include "UI/Cards/CardSelectionWidget.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardPoolConfig.h"
#include "Cards/CardSubsystem.h"
#include "Cards/CardTypes.h"
#include "UI/Cards/CardWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/PanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UCardSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( ConfirmButton )
	{
		ConfirmButton->OnClicked.AddDynamic( this, &UCardSelectionWidget::HandleConfirmClicked );
		ConfirmButton->SetIsEnabled( false );
	}

	if ( RerollButton )
	{
		RerollButton->OnClicked.AddDynamic( this, &UCardSelectionWidget::HandleRerollClicked );
	}
}

void UCardSelectionWidget::NativeDestruct()
{
	if ( IsValid( ConfirmButton ) )
	{
		ConfirmButton->OnClicked.RemoveDynamic( this, &UCardSelectionWidget::HandleConfirmClicked );
	}

	if ( IsValid( RerollButton ) )
	{
		RerollButton->OnClicked.RemoveDynamic( this, &UCardSelectionWidget::HandleRerollClicked );
	}

	ClearCardWidgets();

	Super::NativeDestruct();
}

void UCardSelectionWidget::ShowWithChoices( const FCardChoice& choice )
{
	CurrentWaveNumber_ = choice.WaveNumber;

	TArray<UCardDataAsset*> cards;
	for ( UCardDataAsset* card : choice.AvailableCards )
	{
		cards.Add( card );
	}

	ShowWithCards( cards, choice.CardsToSelect );
}

void UCardSelectionWidget::ShowWithCards( const TArray<UCardDataAsset*>& cards, int32 numToSelect )
{
	CardsToSelect_ = FMath::Max( 1, numToSelect );
	SelectedCardWidgets_.Empty();
	RerollCount_ = 0;

	ClearCardWidgets();
	CreateCardWidgets( cards );

	UpdateTitleText();
	UpdateSelectionUI();
	UpdateRerollUI();

	if ( !IsInViewport() )
	{
		AddToViewport( ViewportZOrder );
	}

	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		FInputModeUIOnly inputMode;
		inputMode.SetWidgetToFocus( TakeWidget() );
		inputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
		pc->SetInputMode( inputMode );
		pc->SetShowMouseCursor( true );
	}

	OnWidgetShown();
}

void UCardSelectionWidget::Hide()
{
	OnWidgetHiding();

	const UWorld* world = GetWorld();
	if ( world && !world->bIsTearingDown )
	{
		if ( APlayerController* pc = UGameplayStatics::GetPlayerController( world, 0 ) )
		{
			FInputModeGameAndUI inputMode;
			inputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
			pc->SetInputMode( inputMode );
		}
	}

	SelectedCardWidgets_.Empty();
	ClearCardWidgets();

	RemoveFromParent();
}

TArray<UCardDataAsset*> UCardSelectionWidget::GetSelectedCards() const
{
	TArray<UCardDataAsset*> result;

	for ( const TObjectPtr<UCardWidget>& widget : SelectedCardWidgets_ )
	{
		if ( widget && widget->GetCardData() )
		{
			result.Add( widget->GetCardData() );
		}
	}

	return result;
}

void UCardSelectionWidget::CreateCardWidgets( const TArray<UCardDataAsset*>& cards )
{
	if ( !CardWidgetClass && RarityCardWidgetClasses.Num() == 0 )
	{
		UE_LOG( LogTemp, Error, TEXT( "CardSelectionWidget: CardWidgetClass not set!" ) );
		return;
	}

	if ( !CardsScrollBox && !CardsContainer )
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "CardSelectionWidget: no CardsScrollBox or CardsContainer bound in the widget blueprint" ) );
		return;
	}

	if ( CardsScrollBox )
	{
		CardsScrollBox->SetOrientation( Orient_Horizontal );
		CardsScrollBox->ClearChildren();
	}

	for ( UCardDataAsset* cardData : cards )
	{
		if ( !cardData )
		{
			continue;
		}

		TSubclassOf<UCardWidget> widgetClass = CardWidgetClass;
		if ( const TSubclassOf<UCardWidget>* rarityClass = RarityCardWidgetClasses.Find( cardData->Rarity );
		     rarityClass && *rarityClass )
		{
			widgetClass = *rarityClass;
		}

		if ( !widgetClass )
		{
			UE_LOG( LogTemp, Warning,
				TEXT( "CardSelectionWidget: no widget class for rarity %d and no fallback CardWidgetClass set" ),
				static_cast<int32>( cardData->Rarity ) );
			continue;
		}

		UCardWidget* cardWidget = CreateWidget<UCardWidget>( this, widgetClass );
		if ( !cardWidget )
		{
			continue;
		}

		cardWidget->SetCardData( cardData );
		cardWidget->OnCardClicked.AddDynamic( this, &UCardSelectionWidget::HandleCardClicked );

		if ( CardsScrollBox )
		{
			UPanelSlot* panelSlot = CardsScrollBox->AddChild( cardWidget );
			if ( UScrollBoxSlot* slot = Cast<UScrollBoxSlot>( panelSlot ) )
			{
				slot->SetPadding( FMargin( CardSpacing * 0.5f, 0.0f ) );
				slot->SetHorizontalAlignment( HAlign_Center );
				slot->SetVerticalAlignment( VAlign_Center );
			}
		}
		else if ( CardsContainer )
		{
			UHorizontalBoxSlot* slot = CardsContainer->AddChildToHorizontalBox( cardWidget );
			if ( slot )
			{
				slot->SetPadding( FMargin( CardSpacing * 0.5f, 0.0f ) );
				slot->SetHorizontalAlignment( HAlign_Center );
				slot->SetVerticalAlignment( VAlign_Center );
			}
		}

		CardWidgets_.Add( cardWidget );
	}
}

void UCardSelectionWidget::ClearCardWidgets()
{
	const UWorld* world = GetWorld();
	const bool bIsTearingDown = !world || world->bIsTearingDown;

	for ( TObjectPtr<UCardWidget>& widget : CardWidgets_ )
	{
		if ( !IsValid( widget ) )
		{
			continue;
		}

		widget->OnCardClicked.RemoveDynamic( this, &UCardSelectionWidget::HandleCardClicked );

		if ( !bIsTearingDown )
		{
			widget->RemoveFromParent();
		}
	}

	CardWidgets_.Empty();
	SelectedCardWidgets_.Empty();
}

void UCardSelectionWidget::HandleCardClicked( UCardWidget* cardWidget )
{
	if ( !cardWidget )
	{
		return;
	}

	if ( cardWidget->IsSelected() )
	{
		cardWidget->SetSelected( false );
		SelectedCardWidgets_.Remove( cardWidget );
		UpdateSelectionUI();
		return;
	}

	for ( const TObjectPtr<UCardWidget>& selected : SelectedCardWidgets_ )
	{
		if ( selected && selected != cardWidget )
		{
			selected->SetSelected( false );
		}
	}
	SelectedCardWidgets_.Empty();

	cardWidget->SetSelected( true );
	SelectedCardWidgets_.Add( cardWidget );

	UpdateSelectionUI();
}

void UCardSelectionWidget::HandleRerollClicked()
{
	UCardSubsystem* cardSubsystem = GetCardSubsystem();
	if ( !cardSubsystem )
	{
		return;
	}

	FCardChoice newChoice;
	if ( !cardSubsystem->TryRerollCardChoice( CurrentWaveNumber_, RerollCount_, newChoice ) )
	{
		UpdateRerollUI();
		return;
	}

	++RerollCount_;

	TArray<UCardDataAsset*> cards;
	cards.Reserve( newChoice.AvailableCards.Num() );
	for ( UCardDataAsset* card : newChoice.AvailableCards )
	{
		cards.Add( card );
	}

	SelectedCardWidgets_.Empty();
	ClearCardWidgets();
	CreateCardWidgets( cards );

	UpdateSelectionUI();
	UpdateRerollUI();

	OnCardsRerolled();
}

void UCardSelectionWidget::HandleConfirmClicked()
{
	if ( !CanConfirm() )
	{
		return;
	}

	TArray<UCardDataAsset*> selectedCards = GetSelectedCards();

	if ( bAutoApplyCards )
	{
		if ( UCardSubsystem* cardSubsystem = GetCardSubsystem() )
		{
			cardSubsystem->ApplySelectedCards( selectedCards );
		}
	}

	OnSelectionComplete.Broadcast( selectedCards );

	Hide();
}

void UCardSelectionWidget::UpdateSelectionUI()
{
	const int32 currentCount = SelectedCardWidgets_.Num();
	const int32 requiredCount = CardsToSelect_;

	if ( ConfirmButton )
	{
		ConfirmButton->SetIsEnabled( currentCount >= requiredCount );
	}

	if ( SelectionCountText )
	{
		FText countText = FText::Format(
			SelectionCountFormat,
			FText::AsNumber( currentCount ),
			FText::AsNumber( requiredCount )
		);
		SelectionCountText->SetText( countText );
	}

	OnSelectionCountChanged( currentCount, requiredCount );
}

void UCardSelectionWidget::UpdateRerollUI()
{
	UCardSubsystem* cardSubsystem = GetCardSubsystem();
	const UCardPoolConfig* poolConfig = cardSubsystem ? cardSubsystem->GetPoolConfig() : nullptr;

	const bool bRerollEnabled = cardSubsystem && cardSubsystem->IsRerollEnabled();
	const bool bMaxReached = poolConfig && poolConfig->MaxRerollsPerSelection > 0
	                         && RerollCount_ >= poolConfig->MaxRerollsPerSelection;
	const bool bCanAfford = bRerollEnabled && !bMaxReached
	                        && cardSubsystem->CanAffordReroll( RerollCount_ );

	const int32 cost = cardSubsystem ? cardSubsystem->GetRerollCost( RerollCount_ ) : 0;

	if ( RerollButton )
	{
		RerollButton->SetVisibility( bRerollEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
		RerollButton->SetIsEnabled( bCanAfford );
	}

	if ( RerollCostText )
	{
		if ( bRerollEnabled && poolConfig )
		{
			RerollCostText->SetText( FText::Format( RerollCostFormat, FText::AsNumber( cost ) ) );
			RerollCostText->SetVisibility( ESlateVisibility::Visible );
		}
		else
		{
			RerollCostText->SetVisibility( ESlateVisibility::Collapsed );
		}
	}

	OnRerollAvailabilityChanged( bCanAfford, cost );
}

void UCardSelectionWidget::UpdateTitleText()
{
	if ( TitleText )
	{
		FText title = FText::Format( TitleFormat, FText::AsNumber( CardsToSelect_ ) );
		TitleText->SetText( title );
	}

	if ( WaveText )
	{
		FText waveTextValue = FText::FromString( FString::Printf( TEXT( "Wave %d" ), CurrentWaveNumber_ ) );
		WaveText->SetText( waveTextValue );
	}
}

UCardSubsystem* UCardSelectionWidget::GetCardSubsystem()
{
	if ( CachedCardSubsystem_.IsValid() )
	{
		return CachedCardSubsystem_.Get();
	}

	CachedCardSubsystem_ = UCardSubsystem::Get( this );
	return CachedCardSubsystem_.Get();
}