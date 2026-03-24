#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Building/Bonus/BuildingBonusComponent.h"
#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/ResourceBuilding.h"
#include "UI/Widgets/BuildingUIConfig.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

//logic mini-widgeth
void UBuildingTooltipResourceRow::Setup( int32 Amount, UTexture2D* Icon, FSlateColor TextColor, bool bShowTurnSuffix )
{
	FString Sign = ( TextColor.GetSpecifiedColor() == FLinearColor::White ) ? TEXT( "" )
	                                                                        : ( Amount > 0 ? TEXT( "+" ) : TEXT( "" ) );
	if ( Text_Amount )
	{
		Text_Amount->SetText( FText::FromString( FString::Printf( TEXT( "%s%d" ), *Sign, Amount ) ) );
		Text_Amount->SetColorAndOpacity( TextColor );
	}
	if ( Img_ResourceIcon && Icon )
		Img_ResourceIcon->SetBrushFromTexture( Icon );
	if ( Text_Suffix )
	{
		Text_Suffix->SetText( FText::FromString( TEXT( "/move" ) ) ); //TODO - language Russian!!!
		Text_Suffix->SetVisibility(
		    bShowTurnSuffix ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed
		);
	}
}

void UBuildingTooltipStatRow::Setup( UTexture2D* Icon, const FString& StatName, const FString& Value )
{
	if ( Img_StatIcon && Icon )
		Img_StatIcon->SetBrushFromTexture( Icon );
	if ( Text_StatName )
		Text_StatName->SetText( FText::FromString( StatName ) );
	if ( Text_Value )
		Text_Value->SetText( FText::FromString( Value ) );
}

void UBuildingTooltipBonusRow::Setup(
    UTexture2D* TargetIcon, UTexture2D* SourceIcon, float Value, UTexture2D* ResourceIcon
)
{
	if ( Img_TargetBuilding && TargetIcon )
		Img_TargetBuilding->SetBrushFromTexture( TargetIcon );
	if ( Img_SourceBuilding && SourceIcon )
		Img_SourceBuilding->SetBrushFromTexture( SourceIcon );
	if ( Img_ResourceIcon && ResourceIcon )
		Img_ResourceIcon->SetBrushFromTexture( ResourceIcon );

	FString Sign = Value > 0 ? TEXT( "+" ) : TEXT( "" );
	if ( Text_Value )
		Text_Value->SetText( FText::FromString( FString::Printf( TEXT( "%s%.0f" ), *Sign, Value ) ) );
}

//logic main widgeth

void UBuildingTooltipWidget::NativeTick( const FGeometry& MyGeometry, float InDeltaTime )
{
	Super::NativeTick( MyGeometry, InDeltaTime );

	switch ( CurrentState )
	{
	case ETooltipState::DelayShow:
		StateTimer -= InDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			if ( CurrentBuildingClass != PendingBuildingClass )
			{
				CurrentBuildingClass = PendingBuildingClass;
				UpdateContent();
			}

			CurrentState = ETooltipState::AnimatingIn;
			SetVisibility( ESlateVisibility::HitTestInvisible );
		}
		break;

	case ETooltipState::AnimatingIn:
		AnimProgress = FMath::Clamp( AnimProgress + ( InDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();

		if ( AnimProgress >= 1.0f )
		{
			CurrentState = ETooltipState::HoldFlash;
			StateTimer = FlashHoldDuration;
		}
		break;

	case ETooltipState::HoldFlash:
		StateTimer -= InDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			CurrentState = ETooltipState::FadeFlash;
			FlashProgress = 1.0f;
		}
		break;

	case ETooltipState::FadeFlash:
		FlashProgress = FMath::Clamp( FlashProgress - ( InDeltaTime / AnimDuration ), 0.0f, 1.0f );
		if ( WhiteFlash )
			WhiteFlash->SetRenderOpacity( FlashProgress );

		if ( FlashProgress <= 0.0f )
		{
			CurrentState = ETooltipState::Visible;
		}
		break;

	case ETooltipState::DelayHide:

		StateTimer -= InDeltaTime;
		if ( StateTimer <= 0.0f )
			CurrentState = ETooltipState::AnimatingOut;
		break;

	case ETooltipState::AnimatingOut:

		AnimProgress = FMath::Clamp( AnimProgress - ( InDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();

		if ( AnimProgress <= 0.0f )
			ForceHide();
		break;

	case ETooltipState::Visible:
	case ETooltipState::Hidden:
		break;
	}
}

void UBuildingTooltipWidget::ShowTooltip( TSubclassOf<ABuilding> BuildingClass, bool bInstantSwitch )
{
	if ( !BuildingClass )
		return;

	PendingBuildingClass = BuildingClass;

	SetVisibility( ESlateVisibility::HitTestInvisible );

	if ( CurrentState == ETooltipState::Hidden || CurrentState == ETooltipState::AnimatingOut ||
	     CurrentState == ETooltipState::DelayHide )
	{
		CurrentState = ETooltipState::DelayShow;
		StateTimer = ShowDelay;
		AnimProgress = 0.0f;
		SetRenderOpacity( 0.0f );
	}
	else if ( CurrentState == ETooltipState::Visible || CurrentState == ETooltipState::FadeFlash ||
	          CurrentState == ETooltipState::HoldFlash )
	{
		if ( WhiteFlash )
			WhiteFlash->SetRenderOpacity( 1.0f );

		if ( CurrentBuildingClass != PendingBuildingClass )
		{
			CurrentBuildingClass = PendingBuildingClass;
			UpdateContent();
		}

		CurrentState = ETooltipState::HoldFlash;
		StateTimer = SwitchDelay;
		AnimProgress = 1.0f;
	}
}

void UBuildingTooltipWidget::HideTooltip()
{
	if ( CurrentState != ETooltipState::Hidden && CurrentState != ETooltipState::AnimatingOut &&
	     CurrentState != ETooltipState::DelayHide )
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
	if ( !AnimationContainer || !WhiteFlash )
		return;
	float alpha = AnimationCurve ? AnimationCurve->GetFloatValue( AnimProgress ) : AnimProgress;
	AnimationContainer->SetRenderTranslation( FVector2D( FMath::Lerp( SlideOffsetX, 0.0f, alpha ), 0.0f ) );
	SetRenderOpacity( alpha );

	if ( CurrentState == ETooltipState::AnimatingIn || CurrentState == ETooltipState::AnimatingOut )
	{
		WhiteFlash->SetRenderOpacity( 1.0f );
	}
}

UTexture2D* UBuildingTooltipWidget::GetResourceIcon( EResourceType Type )
{
	if ( UIConfig && UIConfig->ResourceIcons.Contains( Type ) )
		return UIConfig->ResourceIcons[Type];
	return nullptr;
}

UTexture2D* UBuildingTooltipWidget::GetStatIcon( EStatsType Type )
{
	if ( UIConfig && UIConfig->StatIcons.Contains( Type ) )
		return UIConfig->StatIcons[Type];
	return nullptr;
}

void UBuildingTooltipWidget::UpdateContent()
{
	if ( !CurrentBuildingClass )
		return;
	const ABuilding* cDO = CurrentBuildingClass->GetDefaultObject<ABuilding>();
	if ( !cDO )
		return;

	//clean
	if ( Box_Cost )
		Box_Cost->ClearChildren();
	if ( Box_Maintenance )
		Box_Maintenance->ClearChildren();
	if ( Box_Production )
		Box_Production->ClearChildren();
	if ( Box_Stats )
		Box_Stats->ClearChildren();
	if ( Box_Bonus )
		Box_Bonus->ClearChildren();

	//name and icon
	UTexture2D* BuildingIconTex = nullptr;
	if ( UIConfig && UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		const FBuildingUIData& data = UIConfig->BuildingsData[CurrentBuildingClass];
		Text_Name->SetText( data.Name );
		Text_Description->SetText( data.Description );
		BuildingIconTex = data.Icon;
		if ( Img_Icon )
			Img_Icon->SetBrushFromTexture( data.Icon );
	}
	else
	{
		Text_Name->SetText( FText::FromString( const_cast<ABuilding*>( cDO )->GetNameBuild() ) );
		Text_Description->SetText( FText::FromString( TEXT( "No description" ) ) );
	}

	APlayerController* PC = GetOwningPlayer();
	if ( !PC )
		return;

	//economy
	int32 costG = cDO->GetBuildingCost().Gold;
	int32 costF = cDO->GetBuildingCost().Food;
	int32 costP = cDO->GetBuildingCost().Population;

	int32 maintG = cDO->GetMaintenanceCost().Gold;
	int32 maintF = cDO->GetMaintenanceCost().Food;
	int32 maintP = cDO->GetMaintenanceCost().Population;

	int32 prodG = 0, prodF = 0, prodP = 0;
	if ( const AResourceBuilding* resB = Cast<AResourceBuilding>( cDO ) )
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

	auto AddResRow = [&]( UPanelWidget* Box, int32 Amount, EResourceType ResType, FSlateColor Color, bool bSuffix )
	{
		if ( Amount != 0 && ResourceRowClass && Box )
		{
			UBuildingTooltipResourceRow* Row = CreateWidget<UBuildingTooltipResourceRow>( PC, ResourceRowClass );
			Row->Setup( Amount, GetResourceIcon( ResType ), Color, bSuffix );
			Box->AddChild( Row );
		}
	};

	AddResRow( Box_Cost, costG, EResourceType::Gold, NeutralColor, false );
	AddResRow( Box_Cost, costF, EResourceType::Food, NeutralColor, false );
	AddResRow( Box_Cost, costP, EResourceType::Population, NeutralColor, false );

	AddResRow( Box_Maintenance, -maintG, EResourceType::Gold, ExpenseColor, true );
	AddResRow( Box_Maintenance, -maintF, EResourceType::Food, ExpenseColor, true );
	AddResRow( Box_Maintenance, -maintP, EResourceType::Population, ExpenseColor, false );

	AddResRow( Box_Production, prodG, EResourceType::Gold, IncomeColor, true );
	AddResRow( Box_Production, prodF, EResourceType::Food, IncomeColor, true );
	AddResRow( Box_Production, prodP, EResourceType::Population, IncomeColor, false );

	//stats
	FEntityStats stats = const_cast<ABuilding*>( cDO )->Stats();

	auto AddStatRow = [&]( EStatsType Type, const FString& Name, const FString& Value )
	{
		if ( StatRowClass && Box_Stats )
		{
			UBuildingTooltipStatRow* Row = CreateWidget<UBuildingTooltipStatRow>( PC, StatRowClass );
			Row->Setup( GetStatIcon( Type ), Name, Value );
			Box_Stats->AddChild( Row );
		}
	};

	AddStatRow( EStatsType::MaxHealth, TEXT( "strength" ), FString::FromInt( stats.MaxHealth() ) );

	if ( cDO->IsA<ADefensiveBuilding>() )
	{
		AddStatRow( EStatsType::AttackDamage, TEXT( "Damage" ), FString::FromInt( stats.AttackDamage() ) );
		AddStatRow(
		    EStatsType::AttackCooldown, TEXT( "Value damage" ), FString::Printf( TEXT( "%.1f" ), stats.AttackCooldown() )
		);
		AddStatRow( EStatsType::AttackRange, TEXT( "radius" ), FString::Printf( TEXT( "%.0f" ), stats.AttackRange() ) );
		AddStatRow(
		    EStatsType::SplashRadius, TEXT( "area damage" ),
		    stats.SplashRadius() > 0.0f ? TEXT( "Yes" ) : TEXT( "No" )
		);
	}

	//bonus
	UBuildingBonusComponent* bonusComp = UBuildingBonusComponent::FindInBlueprintClass( CurrentBuildingClass );
	if ( bonusComp && bonusComp->GetBonusEntries().Num() > 0 && BonusRowClass && Box_Bonus )
	{
		for ( const FBuildingBonusEntry& Entry : bonusComp->GetBonusEntries() )
		{
			UTexture2D* SourceIcon = nullptr;
			if ( Entry.SourceBuildingClass )
			{
				if ( UIConfig && UIConfig->BuildingsData.Contains( Entry.SourceBuildingClass ) )
				{
					SourceIcon = UIConfig->BuildingsData[Entry.SourceBuildingClass].Icon;
				}
			}

			UTexture2D* ResIcon = nullptr;
			if ( Entry.Category == EBonusCategory::Production || Entry.Category == EBonusCategory::Maintenance )
			{
				ResIcon = GetResourceIcon( Entry.ResourceType );
			}

			UBuildingTooltipBonusRow* Row = CreateWidget<UBuildingTooltipBonusRow>( PC, BonusRowClass );
			Row->Setup( BuildingIconTex, SourceIcon, Entry.Value, ResIcon );
			Box_Bonus->AddChild( Row );
		}
	}
}