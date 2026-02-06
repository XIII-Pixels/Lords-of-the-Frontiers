#include "Resources/ResourceBalanceWidget.h"

#include "Components/TextBlock.h"
#include "Core/CoreManager.h"
#include "Resources/EconomyComponent.h"

void UResourceBalanceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
	{
		UE_LOG( LogTemp, Warning, TEXT( "ResourceBalanceWidget: CoreManager not available" ) );
		return;
	}

	CachedEconomy_ = Core->GetEconomyComponent();
	if ( !CachedEconomy_ )
	{
		UE_LOG( LogTemp, Warning, TEXT( "ResourceBalanceWidget: EconomyComponent not available" ) );
		return;
	}

	CachedEconomy_->OnEconomyBalanceChanged.AddDynamic(
		this, &UResourceBalanceWidget::OnBalanceChanged );

	OnBalanceChanged( CachedEconomy_->GetNetIncome() );
}

void UResourceBalanceWidget::NativeDestruct()
{
	if ( CachedEconomy_ )
	{
		CachedEconomy_->OnEconomyBalanceChanged.RemoveDynamic(
			this, &UResourceBalanceWidget::OnBalanceChanged );
		CachedEconomy_ = nullptr;
	}

	Super::NativeDestruct();
}

void UResourceBalanceWidget::OnBalanceChanged( const FResourceProduction& NetIncome )
{
	UpdateDisplay( Text_Gold_Net,     NetIncome.Gold );
	UpdateDisplay( Text_Food_Net,     NetIncome.Food );
	UpdateDisplay( Text_Pop_Net,      NetIncome.Population );
	UpdateDisplay( Text_Progress_Net, NetIncome.Progress );
}

void UResourceBalanceWidget::UpdateDisplay( UTextBlock* TextBlock, int32 Value )
{
	if ( !TextBlock )
	{
		return;
	}

	FString DisplayStr;
	FSlateColor Color;

	if ( Value > 0 )
	{
		DisplayStr = FString::Printf( TEXT( "+%d" ), Value );
		Color = FSlateColor( FLinearColor( 0.2f, 0.85f, 0.2f ) ); // Green
	}
	else if ( Value < 0 )
	{
		DisplayStr = FString::Printf( TEXT( "%d" ), Value ); // minus already in number
		Color = FSlateColor( FLinearColor( 0.9f, 0.15f, 0.15f ) ); // Red
	}
	else
	{
		DisplayStr = TEXT( "0" );
		Color = FSlateColor( FLinearColor( 0.5f, 0.5f, 0.5f ) ); // Gray
	}

	TextBlock->SetText( FText::FromString( DisplayStr ) );
	TextBlock->SetColorAndOpacity( Color );
}
