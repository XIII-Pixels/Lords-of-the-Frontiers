#include "Lords_Frontiers/Public/UI/Widgets/HUDResourcePanelWidget.h"

#include "Core/CoreManager.h"
#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"

void UHUDResourcePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TrySubscribeEconomy();

	InitIncomeDisplay();
	UpdateResources();
}

void UHUDResourcePanelWidget::NativeDestruct()
{
	UnsubscribeEconomy();
	Super::NativeDestruct();
}

void UHUDResourcePanelWidget::NativeTick( const FGeometry& MyGeometry, float InDeltaTime )
{
	Super::NativeTick( MyGeometry, InDeltaTime );

	if ( !bIsEconomySubscribed_ )
	{
		if ( TrySubscribeEconomy() )
		{
			UpdateResources();
			InitIncomeDisplay();
		}
	}

	TickIncomeAnimation( Text_GoldIncome, Arrow_Gold, GoldIncomeAnim_, InDeltaTime );
	TickIncomeAnimation( Text_FoodIncome, Arrow_Food, FoodIncomeAnim_, InDeltaTime );

	TickResourceAnimation( GoldAnim_, Text_Gold, InDeltaTime );
	TickResourceAnimation( FoodAnim_, Text_Food, InDeltaTime );
	TickResourceAnimation( CitizensAnim_, Text_Citizens, InDeltaTime );
}

bool UHUDResourcePanelWidget::TrySubscribeEconomy()
{
	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return false;
	}

	UResourceManager* rM = core->GetResourceManager();
	UEconomyComponent* eC = core->GetEconomyComponent();
	if ( !rM || !eC )
	{
		return false;
	}

	rM->OnResourceChanged.AddUniqueDynamic( this, &UHUDResourcePanelWidget::HandleResourceChanged );
	eC->OnNetIncomeChanged.AddUniqueDynamic( this, &UHUDResourcePanelWidget::HandleNetIncomeChanged );
	bIsEconomySubscribed_ = true;
	return true;
}

void UHUDResourcePanelWidget::UnsubscribeEconomy()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UResourceManager* rM = core->GetResourceManager() )
		{
			rM->OnResourceChanged.RemoveDynamic( this, &UHUDResourcePanelWidget::HandleResourceChanged );
		}
		if ( UEconomyComponent* eC = core->GetEconomyComponent() )
		{
			eC->OnNetIncomeChanged.RemoveDynamic( this, &UHUDResourcePanelWidget::HandleNetIncomeChanged );
		}
	}
	bIsEconomySubscribed_ = false;
}

void UHUDResourcePanelWidget::HandleResourceChanged( EResourceType Type, int32 NewAmount )
{
	UpdateResources();
}

void UHUDResourcePanelWidget::UpdateResources()
{
	UCoreManager* core = UCoreManager::Get( this );
	UResourceManager* rM = core ? core->GetResourceManager() : nullptr;
	if ( !rM )
	{
		return;
	}

	SetResourceTarget( GoldAnim_, Text_Gold, rM->GetResourceAmount( EResourceType::Gold ) );
	SetResourceTarget( FoodAnim_, Text_Food, rM->GetResourceAmount( EResourceType::Food ) );
	SetResourceTarget( CitizensAnim_, Text_Citizens, rM->GetResourceAmount( EResourceType::Population ) );
}

void UHUDResourcePanelWidget::SetResourceTarget( FResourceAnimState& state, UTextBlock* textBlock, int32 newValue )
{
	if ( newValue == state.TargetValue && !state.bAnimating )
	{
		if ( textBlock && state.DisplayedValue != newValue )
		{
			state.DisplayedValue = newValue;
			textBlock->SetText( FText::AsNumber( newValue ) );
		}
		return;
	}

	state.StartValue = state.DisplayedValue;
	state.TargetValue = newValue;
	state.Elapsed = 0.0f;
	state.bAnimating = ( state.StartValue != state.TargetValue );

	if ( !state.bAnimating )
	{
		state.DisplayedValue = newValue;
	}

	if ( textBlock )
	{
		textBlock->SetText( FText::AsNumber( state.DisplayedValue ) );
	}
}

void UHUDResourcePanelWidget::TickResourceAnimation( FResourceAnimState& state, UTextBlock* textBlock, float deltaTime )
{
	if ( !state.bAnimating )
	{
		return;
	}

	state.Elapsed += deltaTime;
	const float duration = FMath::Max( ResourceAnimationDuration, 0.01f );
	const float alpha = FMath::Clamp( state.Elapsed / duration, 0.0f, 1.0f );

	state.DisplayedValue = FMath::RoundToInt(
	    FMath::Lerp( static_cast<float>( state.StartValue ), static_cast<float>( state.TargetValue ), alpha )
	);

	if ( textBlock )
	{
		textBlock->SetText( FText::AsNumber( state.DisplayedValue ) );
	}

	if ( alpha >= 1.0f )
	{
		state.bAnimating = false;
		state.DisplayedValue = state.TargetValue;
		if ( textBlock )
		{
			textBlock->SetText( FText::AsNumber( state.DisplayedValue ) );
		}
	}
}

void UHUDResourcePanelWidget::InitIncomeDisplay()
{
	if ( Arrow_Gold )
	{
		Arrow_Gold->SetVisibility( ESlateVisibility::Collapsed );
	}
	if ( Arrow_Food )
	{
		Arrow_Food->SetVisibility( ESlateVisibility::Collapsed );
	}

	ApplyIncomeText( Text_GoldIncome, 0 );
	ApplyIncomeText( Text_FoodIncome, 0 );

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UEconomyComponent* eC = core->GetEconomyComponent() )
		{
			eC->OnNetIncomeChanged.RemoveDynamic( this, &UHUDResourcePanelWidget::HandleNetIncomeChanged );
			eC->OnNetIncomeChanged.AddDynamic( this, &UHUDResourcePanelWidget::HandleNetIncomeChanged );

			FResourceProduction netIncome = eC->CalculateNetIncome();
			GoldIncomeAnim_.DisplayedValue = netIncome.Gold;
			GoldIncomeAnim_.TargetValue = netIncome.Gold;
			FoodIncomeAnim_.DisplayedValue = netIncome.Food;
			FoodIncomeAnim_.TargetValue = netIncome.Food;

			ApplyIncomeText( Text_GoldIncome, netIncome.Gold );
			ApplyIncomeText( Text_FoodIncome, netIncome.Food );
		}
	}
}

void UHUDResourcePanelWidget::HandleNetIncomeChanged( const FResourceProduction& netIncome )
{
	StartIncomeAnimation( Text_GoldIncome, Arrow_Gold, GoldIncomeAnim_, netIncome.Gold );
	StartIncomeAnimation( Text_FoodIncome, Arrow_Food, FoodIncomeAnim_, netIncome.Food );
}

void UHUDResourcePanelWidget::StartIncomeAnimation(
    UTextBlock* textBlock, UImage* arrow, FIncomeAnimState& state, int32 newValue
)
{
	if ( newValue == state.TargetValue )
	{
		return;
	}

	state.StartValue = state.DisplayedValue;
	state.TargetValue = newValue;
	state.Elapsed = 0.0f;
	state.bAnimating = true;
	state.ArrowTimer = ArrowDisplayDuration;

	ApplyIncomeText( textBlock, state.DisplayedValue );

	if ( arrow )
	{
		bool bIncrease = ( newValue > state.StartValue );
		UTexture2D* arrowTexture = bIncrease ? ArrowUpTexture.Get() : ArrowDownTexture.Get();

		if ( arrowTexture )
		{
			arrow->SetBrushFromTexture( arrowTexture );
		}

		arrow->SetColorAndOpacity(
		    bIncrease ? PositiveIncomeColor.GetSpecifiedColor() : NegativeIncomeColor.GetSpecifiedColor()
		);
		arrow->SetRenderOpacity( 1.0f );
		arrow->SetVisibility( ESlateVisibility::HitTestInvisible );
	}
}

void UHUDResourcePanelWidget::TickIncomeAnimation(
    UTextBlock* textBlock, UImage* arrow, FIncomeAnimState& state, float deltaTime
)
{
	const float cArrowFadeDuration = 0.5f;

	if ( state.bAnimating )
	{
		state.Elapsed += deltaTime;
		float alpha = FMath::Clamp( state.Elapsed / FMath::Max( IncomeAnimationDuration, 0.01f ), 0.0f, 1.0f );

		state.DisplayedValue = FMath::RoundToInt(
		    FMath::Lerp( static_cast<float>( state.StartValue ), static_cast<float>( state.TargetValue ), alpha )
		);

		ApplyIncomeText( textBlock, state.DisplayedValue );

		if ( alpha >= 1.0f )
		{
			state.bAnimating = false;
			state.DisplayedValue = state.TargetValue;
			ApplyIncomeText( textBlock, state.DisplayedValue );
		}
	}

	if ( state.ArrowTimer > 0.0f )
	{
		state.ArrowTimer -= deltaTime;

		if ( state.ArrowTimer <= 0.0f && arrow )
		{
			arrow->SetVisibility( ESlateVisibility::Collapsed );
		}
		else if ( arrow && state.ArrowTimer < cArrowFadeDuration )
		{
			float fadeAlpha = FMath::Max( 0.0f, state.ArrowTimer / cArrowFadeDuration );
			arrow->SetRenderOpacity( fadeAlpha );
		}
	}
}

void UHUDResourcePanelWidget::ApplyIncomeText( UTextBlock* textBlock, int32 value )
{
	if ( !textBlock )
	{
		return;
	}

	FString displayText;
	if ( value > 0 )
	{
		displayText = FString::Printf( TEXT( "+%d" ), value );
		textBlock->SetColorAndOpacity( PositiveIncomeColor );
	}
	else if ( value < 0 )
	{
		displayText = FString::Printf( TEXT( "%d" ), value );
		textBlock->SetColorAndOpacity( NegativeIncomeColor );
	}
	else
	{
		displayText = TEXT( "0" );
		textBlock->SetColorAndOpacity( FSlateColor( FLinearColor::White ) );
	}

	textBlock->SetText( FText::FromString( displayText ) );
}
