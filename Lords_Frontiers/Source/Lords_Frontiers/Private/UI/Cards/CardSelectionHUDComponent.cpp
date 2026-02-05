#include "UI/Cards/CardSelectionHUDComponent.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardSubsystem.h"
#include "UI/Cards/CardSelectionWidget.h"

UCardSelectionHUDComponent::UCardSelectionHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCardSelectionHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( bAutoSubscribe )
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick( [this]()
		{
			SubscribeToCardSubsystem();
		} );
	}
}

void UCardSelectionHUDComponent::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	UnsubscribeFromCardSubsystem();

	if ( CardSelectionWidget_ )
	{
		CardSelectionWidget_->RemoveFromParent();
		CardSelectionWidget_ = nullptr;
	}

	Super::EndPlay( endPlayReason );
}

void UCardSelectionHUDComponent::ShowCardSelection( const FCardChoice& choice )
{
	EnsureWidgetCreated();

	if ( CardSelectionWidget_ )
	{
		CardSelectionWidget_->ShowWithChoices( choice );
	}
}

void UCardSelectionHUDComponent::HideCardSelection()
{
	if ( CardSelectionWidget_ )
	{
		CardSelectionWidget_->Hide();
	}
}

bool UCardSelectionHUDComponent::IsCardSelectionVisible() const
{
	return CardSelectionWidget_ && CardSelectionWidget_->IsInViewport();
}

void UCardSelectionHUDComponent::SubscribeToCardSubsystem()
{
	if ( bIsSubscribed_ )
	{
		return;
	}

	UCardSubsystem* cardSubsystem = GetCardSubsystem();
	if ( !cardSubsystem )
	{
		UE_LOG( LogTemp, Warning, TEXT( "CardSelectionHUDComponent: CardSubsystem not found!" ) );
		return;
	}

	cardSubsystem->OnCardSelectionRequired.AddDynamic( this, &UCardSelectionHUDComponent::HandleCardSelectionRequired );
	bIsSubscribed_ = true;

	UE_LOG( LogTemp, Log, TEXT( "CardSelectionHUDComponent: Subscribed to CardSubsystem" ) );
}

void UCardSelectionHUDComponent::UnsubscribeFromCardSubsystem()
{
	if ( !bIsSubscribed_ )
	{
		return;
	}

	if ( UCardSubsystem* cardSubsystem = GetCardSubsystem() )
	{
		cardSubsystem->OnCardSelectionRequired.RemoveDynamic( this, &UCardSelectionHUDComponent::HandleCardSelectionRequired );
	}

	bIsSubscribed_ = false;
}

void UCardSelectionHUDComponent::HandleCardSelectionRequired( const FCardChoice& choice )
{
	UE_LOG( LogTemp, Log, TEXT( "CardSelectionHUDComponent: Card selection required. %d cards, select %d" ),
		choice.AvailableCards.Num(), choice.CardsToSelect );

	ShowCardSelection( choice );
}

void UCardSelectionHUDComponent::HandleSelectionComplete( const TArray<UCardDataAsset*>& selectedCards )
{
	UE_LOG( LogTemp, Log, TEXT( "CardSelectionHUDComponent: Selection complete. %d cards selected" ), selectedCards.Num() );
}

UCardSubsystem* UCardSelectionHUDComponent::GetCardSubsystem()
{
	if ( CachedCardSubsystem_.IsValid() )
	{
		return CachedCardSubsystem_.Get();
	}

	CachedCardSubsystem_ = UCardSubsystem::Get( GetOwner() );
	return CachedCardSubsystem_.Get();
}

void UCardSelectionHUDComponent::EnsureWidgetCreated()
{
	if ( CardSelectionWidget_ )
	{
		return;
	}

	if ( !CardSelectionWidgetClass )
	{
		UE_LOG( LogTemp, Error, TEXT( "CardSelectionHUDComponent: CardSelectionWidgetClass not set!" ) );
		return;
	}

	APlayerController* pc = Cast<APlayerController>( GetOwner() );
	if ( !pc )
	{
		pc = GetWorld()->GetFirstPlayerController();
	}

	if ( !pc )
	{
		UE_LOG( LogTemp, Error, TEXT( "CardSelectionHUDComponent: No PlayerController found!" ) );
		return;
	}

	CardSelectionWidget_ = CreateWidget<UCardSelectionWidget>( pc, CardSelectionWidgetClass );

	if ( CardSelectionWidget_ )
	{
		CardSelectionWidget_->OnSelectionComplete.AddDynamic( this, &UCardSelectionHUDComponent::HandleSelectionComplete );
		UE_LOG( LogTemp, Log, TEXT( "CardSelectionHUDComponent: Widget created" ) );
	}
}
