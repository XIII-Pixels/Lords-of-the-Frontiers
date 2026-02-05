#include "UI/Cards/CardSelectionWidget.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardSubsystem.h"
#include "UI/Cards/CardWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
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
}

void UCardSelectionWidget::NativeDestruct()
{
	if ( ConfirmButton )
	{
		ConfirmButton->OnClicked.RemoveDynamic( this, &UCardSelectionWidget::HandleConfirmClicked );
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

	ClearCardWidgets();
	CreateCardWidgets( cards );

	UpdateTitleText();
	UpdateSelectionUI();

	if ( !IsInViewport() )
	{
		AddToViewport( 100 );
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

	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		FInputModeGameAndUI inputMode;
		inputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
		pc->SetInputMode( inputMode );
	}

	RemoveFromParent();

	ClearCardWidgets();
	SelectedCardWidgets_.Empty();
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
	if ( !CardsContainer || !CardWidgetClass )
	{
		UE_LOG( LogTemp, Error, TEXT( "CardSelectionWidget: CardsContainer or CardWidgetClass not set!" ) );
		return;
	}

	for ( UCardDataAsset* cardData : cards )
	{
		if ( !cardData )
		{
			continue;
		}

		UCardWidget* cardWidget = CreateWidget<UCardWidget>( this, CardWidgetClass );
		if ( !cardWidget )
		{
			continue;
		}

		cardWidget->SetCardData( cardData );
		cardWidget->OnCardClicked.AddDynamic( this, &UCardSelectionWidget::HandleCardClicked );

		UHorizontalBoxSlot* slot = CardsContainer->AddChildToHorizontalBox( cardWidget );
		if ( slot )
		{
			slot->SetPadding( FMargin( CardSpacing * 0.5f, 0.0f ) );
			slot->SetHorizontalAlignment( HAlign_Center );
			slot->SetVerticalAlignment( VAlign_Center );
		}

		CardWidgets_.Add( cardWidget );
	}
}

void UCardSelectionWidget::ClearCardWidgets()
{
	for ( TObjectPtr<UCardWidget>& widget : CardWidgets_ )
	{
		if ( widget )
		{
			widget->OnCardClicked.RemoveDynamic( this, &UCardSelectionWidget::HandleCardClicked );
			widget->RemoveFromParent();
		}
	}

	CardWidgets_.Empty();
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
	}
	else
	{
		if ( SelectedCardWidgets_.Num() < CardsToSelect_ )
		{
			cardWidget->SetSelected( true );
			SelectedCardWidgets_.Add( cardWidget );
		}
	}

	UpdateSelectionUI();
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
