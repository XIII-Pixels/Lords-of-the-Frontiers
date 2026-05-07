#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Building/Bonus/BuildingBonusComponent.h"
#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/ResourceBuilding.h"
#include "Core/CoreManager.h"
#include "Resources/ResourceManager.h"
#include "UI/Widgets/BuildingUIConfig.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

// logic mini-widgeth
void UBuildingTooltipResourceRow::Setup(
    int32 amount, UTexture2D* icon, FSlateColor textColor, bool bShowTurnSuffix, bool bShowPlusSign
)
{
	FString sign = ( bShowPlusSign && ( amount > 0 ) ) ? TEXT( "+" ) : TEXT( "" );

	if ( IsValid( Text_Amount ) )
	{
		Text_Amount->SetText( FText::FromString( FString::Printf( TEXT( "%s%d" ), *sign, amount ) ) );
		Text_Amount->SetColorAndOpacity( textColor );
	}
	if ( IsValid( Img_ResourceIcon ) && IsValid( icon ) )
	{
		Img_ResourceIcon->SetBrushFromTexture( icon );
	}
	if ( IsValid( Text_Suffix ) )
	{
		Text_Suffix->SetText( FText::FromString( TEXT( "/ход  " ) ) );
		Text_Suffix->SetVisibility(
		    bShowTurnSuffix ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed
		);
	}
}

void UBuildingTooltipStatRow::Setup( UTexture2D* icon, const FString& statName, const FString& value )
{
	if ( IsValid( Img_StatIcon ) && IsValid( icon ) )
	{
		Img_StatIcon->SetBrushFromTexture( icon );
	}
	if ( IsValid( Text_StatName ) )
	{
		Text_StatName->SetText( FText::FromString( statName ) );
	}
	if ( IsValid( Text_Value ) )
	{
		Text_Value->SetText( FText::FromString( value ) );
	}
}

void UBuildingTooltipHealthRow::Setup( UTexture2D* icon, const FString& healthValue )
{
	if ( IsValid( Img_HealthIcon ) && IsValid( icon ) )
	{
		Img_HealthIcon->SetBrushFromTexture( icon );
	}
	if ( IsValid( Text_HealthValue ) )
	{
		Text_HealthValue->SetText( FText::FromString( healthValue ) );
	}
}

void UBuildingTooltipBonusRow::Setup(
    UTexture2D* targetIcon, UTexture2D* sourceIcon, float value, UTexture2D* resourceIcon
)
{
	if ( IsValid( Img_TargetBuilding ) && IsValid( targetIcon ) )
	{
		Img_TargetBuilding->SetBrushFromTexture( targetIcon );
	}
	if ( IsValid( Img_SourceBuilding ) && IsValid( sourceIcon ) )
	{
		Img_SourceBuilding->SetBrushFromTexture( sourceIcon );
	}
	if ( IsValid( Img_ResourceIcon ) && IsValid( resourceIcon ) )
	{
		Img_ResourceIcon->SetBrushFromTexture( resourceIcon );
	}

	FString sign = ( value > 0 ) ? TEXT( "+" ) : TEXT( "" );

	if ( IsValid( Text_Value ) )
	{
		Text_Value->SetText( FText::FromString( FString::Printf( TEXT( "%s%.0f" ), *sign, value ) ) );
	}
}

// logic main widgeth

void UBuildingTooltipWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	switch ( CurrentState )
	{
	case ETooltipState::DelayShow:
		StateTimer -= inDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			CurrentState = ETooltipState::AnimatingIn;
			SetVisibility( ESlateVisibility::HitTestInvisible );
		}
		break;

	case ETooltipState::AnimatingIn:
		AnimProgress = FMath::Clamp( AnimProgress + ( inDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();

		if ( AnimProgress >= 1.0f )
		{
			CurrentState = ETooltipState::HoldFlash;
			StateTimer = FlashHoldDuration;
		}
		break;

	case ETooltipState::HoldFlash:
		StateTimer -= inDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			CurrentState = ETooltipState::FadeFlash;
			FlashProgress = 1.0f;
		}
		break;

	case ETooltipState::FadeFlash:
		FlashProgress = FMath::Clamp( FlashProgress - ( inDeltaTime / AnimDuration ), 0.0f, 1.0f );
		if ( IsValid( WhiteFlash ) )
		{
			WhiteFlash->SetRenderOpacity( FlashProgress );
		}

		if ( FlashProgress <= 0.0f )
		{
			CurrentState = ETooltipState::Visible;
		}
		break;

	case ETooltipState::DelayHide:
		StateTimer -= inDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			CurrentState = ETooltipState::AnimatingOut;
		}
		break;

	case ETooltipState::AnimatingOut:
		AnimProgress = FMath::Clamp( AnimProgress - ( inDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();

		if ( AnimProgress <= 0.0f )
		{
			ForceHide();
		}
		break;

	case ETooltipState::Visible:
		if ( bIsAutoHiding )
		{
			AutoHideTimer -= inDeltaTime;
			if ( AutoHideTimer <= 0.0f )
			{
				bIsAutoHiding = false;
				HideTooltip();
			}
		}
		break;

	case ETooltipState::Hidden:
		break;
	}
}

void UBuildingTooltipWidget::ShowTooltip( TSubclassOf<ABuilding> buildingClass )
{
	if ( !IsValid( buildingClass ) )
	{
		return;
	}

	bIsAutoHiding = false;
	PendingBuildingClass = buildingClass;

	SetVisibility( ESlateVisibility::HitTestInvisible );

	if ( ( CurrentState == ETooltipState::Hidden ) || ( CurrentState == ETooltipState::AnimatingOut ) ||
	     ( CurrentState == ETooltipState::DelayHide ) )
	{
		CurrentState = ETooltipState::DelayShow;
		StateTimer = ShowDelay;
		AnimProgress = 0.0f;
		SetRenderOpacity( 0.0f );

		// Фикс бага: всегда обновляем контент при показе
		CurrentBuildingClass = PendingBuildingClass;
		UpdateContent();
	}
	else if ( ( CurrentState == ETooltipState::Visible ) || ( CurrentState == ETooltipState::FadeFlash ) ||
	          ( CurrentState == ETooltipState::HoldFlash ) )
	{
		if ( IsValid( WhiteFlash ) )
		{
			WhiteFlash->SetRenderOpacity( 1.0f );
		}

		// Фикс бага: всегда обновляем контент при переключении
		CurrentBuildingClass = PendingBuildingClass;
		UpdateContent();

		CurrentState = ETooltipState::HoldFlash;
		StateTimer = SwitchDelay;
		AnimProgress = 1.0f;
	}
}

void UBuildingTooltipWidget::HideTooltip()
{
	if ( ( CurrentState != ETooltipState::Hidden ) && ( CurrentState != ETooltipState::AnimatingOut ) &&
	     ( CurrentState != ETooltipState::DelayHide ) )
	{
		CurrentState = ETooltipState::DelayHide;
		StateTimer = HideDelay;
	}
}

void UBuildingTooltipWidget::ForceHide()
{
	CurrentState = ETooltipState::Hidden;
	AnimProgress = 0.0f;
	SetRenderOpacity( 0.0f );
	SetVisibility( ESlateVisibility::Hidden );
}

void UBuildingTooltipWidget::ApplyAnimation()
{
	if ( !IsValid( AnimationContainer ) || !IsValid( WhiteFlash ) )
	{
		return;
	}

	float alpha = IsValid( AnimationCurve ) ? AnimationCurve->GetFloatValue( AnimProgress ) : AnimProgress;
	AnimationContainer->SetRenderTranslation( FVector2D( FMath::Lerp( SlideOffsetX, 0.0f, alpha ), 0.0f ) );
	SetRenderOpacity( alpha );

	if ( ( CurrentState == ETooltipState::AnimatingIn ) || ( CurrentState == ETooltipState::AnimatingOut ) )
	{
		WhiteFlash->SetRenderOpacity( 1.0f );
	}
}

UTexture2D* UBuildingTooltipWidget::GetResourceIcon( EResourceType type )
{
	if ( IsValid( UIConfig ) && UIConfig->ResourceIcons.Contains( type ) )
	{
		return UIConfig->ResourceIcons[type];
	}
	return nullptr;
}

UTexture2D* UBuildingTooltipWidget::GetStatIcon( EStatsType type )
{
	if ( IsValid( UIConfig ) && UIConfig->StatIcons.Contains( type ) )
	{
		return UIConfig->StatIcons[type];
	}
	return nullptr;
}

void UBuildingTooltipWidget::UpdateContent()
{
	if ( !IsValid( CurrentBuildingClass ) )
	{
		return;
	}

	const ABuilding* cdo = CurrentBuildingClass->GetDefaultObject<ABuilding>();
	if ( !IsValid( cdo ) )
	{
		return;
	}

	ClearContainers();
	UpdateHeader( cdo );
	UpdateEconomy( cdo );
	UpdateStats( cdo );
	UpdateBonuses();
}

void UBuildingTooltipWidget::ClearContainers()
{
	if ( IsValid( Box_Cost ) )
	{
		Box_Cost->ClearChildren();
	}
	if ( IsValid( Box_Maintenance ) )
	{
		Box_Maintenance->ClearChildren();
	}
	if ( IsValid( Box_Production ) )
	{
		Box_Production->ClearChildren();
	}
	if ( IsValid( Box_Stats ) )
	{
		Box_Stats->ClearChildren();
	}
	if ( IsValid( Box_Bonus ) )
	{
		Box_Bonus->ClearChildren();
	}
	if ( IsValid( Box_Health ) )
	{
		Box_Health->ClearChildren();
	}
}

void UBuildingTooltipWidget::UpdateHeader( const ABuilding* cdo )
{
	if ( IsValid( UIConfig ) && UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		const FBuildingUIData& data = UIConfig->BuildingsData[CurrentBuildingClass];
		Text_Name->SetText( data.Name );
		Text_Description->SetText( data.Description );

		if ( IsValid( Img_Icon ) )
		{
			Img_Icon->SetBrushFromTexture( data.Icon );
		}
	}
	else
	{
		Text_Name->SetText( FText::FromString( const_cast<ABuilding*>( cdo )->GetNameBuild() ) );
		Text_Description->SetText( FText::FromString( TEXT( "Нет описания" ) ) );
	}
}

void UBuildingTooltipWidget::UpdateEconomy( const ABuilding* cdo )
{
	APlayerController* pc = GetOwningPlayer();
	if ( !IsValid( pc ) )
	{
		return;
	}

	int32 playerGold = 0, playerFood = 0, playerPop = 0;
	if ( UCoreManager* core = UCoreManager::Get( pc ) )
	{
		if ( UResourceManager* rM = core->GetResourceManager() )
		{
			playerGold = rM->GetResourceAmount( EResourceType::Gold );
			playerFood = rM->GetResourceAmount( EResourceType::Food );
			playerPop = rM->GetResourceAmount( EResourceType::Population );
		}
	}

	int32 costG = cdo->GetBuildingCost().Gold;
	int32 costF = cdo->GetBuildingCost().Food;
	int32 costP = cdo->GetBuildingCost().Population;

	int32 maintG = cdo->GetMaintenanceCost().Gold;
	int32 maintF = cdo->GetMaintenanceCost().Food;
	int32 maintP = cdo->GetMaintenanceCost().Population;

	int32 prodG = 0, prodF = 0, prodP = 0;
	if ( const AResourceBuilding* resB = Cast<AResourceBuilding>( cdo ) )
	{
		prodG = resB->GetProductionConfig().Gold;
		prodF = resB->GetProductionConfig().Food;
		prodP = resB->GetProductionConfig().Population;
	}

	if ( costP < 0 )
	{
		prodP += FMath::Abs( costP );
		costP = 0;
	}
	if ( maintP < 0 )
	{
		prodP += FMath::Abs( maintP );
		maintP = 0;
	}

	auto addResRow = [&]( UPanelWidget* box, int32 amount, EResourceType resType, FSlateColor color, bool bSuffix,
	                      bool bShowPlus = true )
	{
		if ( ( amount != 0 ) && IsValid( ResourceRowClass ) && IsValid( box ) )
		{
			UBuildingTooltipResourceRow* row = CreateWidget<UBuildingTooltipResourceRow>( pc, ResourceRowClass );
			row->Setup( amount, GetResourceIcon( resType ), color, bSuffix, bShowPlus );
			box->AddChild( row );
		}
	};

	FSlateColor colorG = ( playerGold >= costG ) ? AffordableCostColor : TooExpensiveCostColor;
	FSlateColor colorF = ( playerFood >= costF ) ? AffordableCostColor : TooExpensiveCostColor;
	FSlateColor colorP = ( playerPop >= costP ) ? AffordableCostColor : TooExpensiveCostColor;

	addResRow( Box_Cost, costG, EResourceType::Gold, colorG, false, false );
	addResRow( Box_Cost, costF, EResourceType::Food, colorF, false, false );
	addResRow( Box_Cost, costP, EResourceType::Population, colorP, false, false );

	addResRow( Box_Maintenance, -maintG, EResourceType::Gold, ExpenseColor, true, false );
	addResRow( Box_Maintenance, -maintF, EResourceType::Food, ExpenseColor, true, false );
	addResRow( Box_Maintenance, -maintP, EResourceType::Population, ExpenseColor, false, false );

	addResRow( Box_Production, prodG, EResourceType::Gold, IncomeColor, true );
	addResRow( Box_Production, prodF, EResourceType::Food, IncomeColor, true );
	addResRow( Box_Production, prodP, EResourceType::Population, IncomeColor, false );
}

void UBuildingTooltipWidget::UpdateStats( const ABuilding* cdo )
{
	APlayerController* pc = GetOwningPlayer();
	if ( !IsValid( pc ) )
	{
		return;
	}

	FEntityStats stats = const_cast<ABuilding*>( cdo )->Stats();

	if ( IsValid( HealthRowClass ) && IsValid( Box_Health ) )
	{
		UBuildingTooltipHealthRow* healthRow = CreateWidget<UBuildingTooltipHealthRow>( pc, HealthRowClass );
		healthRow->Setup( GetStatIcon( EStatsType::MaxHealth ), FString::FromInt( stats.MaxHealth() ) );
		Box_Health->AddChild( healthRow );
	}

	if ( cdo->IsA<ADefensiveBuilding>() )
	{
		auto addStatRow = [&]( EStatsType type, const FString& name, const FString& value )
		{
			if ( IsValid( StatRowClass ) && IsValid( Box_Stats ) )
			{
				UBuildingTooltipStatRow* row = CreateWidget<UBuildingTooltipStatRow>( pc, StatRowClass );
				row->Setup( GetStatIcon( type ), name, value );
				Box_Stats->AddChild( row );
			}
		};

		addStatRow( EStatsType::AttackDamage, TEXT( "Урон" ), FString::FromInt( stats.AttackDamage() ) );
		addStatRow(
		    EStatsType::AttackCooldown, TEXT( "Скорость" ), FString::Printf( TEXT( "%.1f" ), stats.AttackCooldown() )
		);
		addStatRow( EStatsType::AttackRange, TEXT( "Радиус" ), FString::Printf( TEXT( "%.0f" ), stats.AttackRange() ) );
		addStatRow(
		    EStatsType::SplashRadius, TEXT( "Область" ), ( stats.SplashRadius() > 0.0f ) ? TEXT( "Да" ) : TEXT( "Нет" )
		);
	}
}

void UBuildingTooltipWidget::UpdateBonuses()
{
	APlayerController* pc = GetOwningPlayer();
	if ( !IsValid( pc ) || !IsValid( BonusRowClass ) || !IsValid( Box_Bonus ) || !IsValid( UIConfig ) )
	{
		return;
	}

	UTexture2D* myBuildingIconTex = nullptr;
	if ( UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		myBuildingIconTex = UIConfig->BuildingsData[CurrentBuildingClass].Icon;
	}

	auto addBonusRow = [&]( UTexture2D* targetIcon, UTexture2D* sourceIcon, float value, EBonusCategory category,
	                        EResourceType resType, EStatsType statType )
	{
		UTexture2D* finalIcon = nullptr;
		if ( category == EBonusCategory::Stats )
		{
			finalIcon = GetStatIcon( statType );
		}
		else
		{
			finalIcon = GetResourceIcon( resType );
		}

		UBuildingTooltipBonusRow* row = CreateWidget<UBuildingTooltipBonusRow>( pc, BonusRowClass );
		row->Setup( targetIcon, sourceIcon, value, finalIcon );
		Box_Bonus->AddChild( row );
	};

	UBuildingBonusComponent* myBonusComp = UBuildingBonusComponent::FindInBlueprintClass( CurrentBuildingClass );
	if ( IsValid( myBonusComp ) )
	{
		for ( const FBuildingBonusEntry& entry : myBonusComp->GetBonusEntries() )
		{
			UTexture2D* sourceIcon = nullptr;
			if ( IsValid( entry.SourceBuildingClass ) && UIConfig->BuildingsData.Contains( entry.SourceBuildingClass ) )
			{
				sourceIcon = UIConfig->BuildingsData[entry.SourceBuildingClass].Icon;
			}

			addBonusRow(
			    myBuildingIconTex, sourceIcon, entry.Value, entry.Category, entry.ResourceType, entry.StatType
			);
		}
	}

	for ( const auto& kvp : UIConfig->BuildingsData )
	{
		TSubclassOf<ABuilding> otherBuildingClass = kvp.Key;
		if ( !IsValid( otherBuildingClass ) || ( otherBuildingClass == CurrentBuildingClass ) )
		{
			continue;
		}

		UBuildingBonusComponent* otherBonusComp = UBuildingBonusComponent::FindInBlueprintClass( otherBuildingClass );
		if ( IsValid( otherBonusComp ) )
		{
			for ( const FBuildingBonusEntry& entry : otherBonusComp->GetBonusEntries() )
			{
				if ( entry.SourceBuildingClass == CurrentBuildingClass )
				{
					UTexture2D* otherBuildingIcon = kvp.Value.Icon;

					addBonusRow(
					    otherBuildingIcon, myBuildingIconTex, entry.Value, entry.Category, entry.ResourceType,
					    entry.StatType
					);
				}
			}
		}
	}
}

void UBuildingTooltipWidget::StartAutoHideTimer()
{
	if ( ( CurrentState == ETooltipState::Visible ) || ( CurrentState == ETooltipState::FadeFlash ) )
	{
		bIsAutoHiding = true;
		AutoHideTimer = AutoHideDelay;
	}
}