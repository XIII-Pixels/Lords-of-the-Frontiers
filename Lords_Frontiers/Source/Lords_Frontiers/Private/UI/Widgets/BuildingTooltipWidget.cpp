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
void UBuildingTooltipResourceRow::Setup(
    int32 amount, UTexture2D* icon, FSlateColor textColor, bool bShowTurnSuffix, bool bShowPlusSign
)
{
	FString Sign = ( bShowPlusSign && amount > 0 ) ? TEXT( "+" ) : TEXT( "" );

	if ( Text_Amount )
	{
		Text_Amount->SetText( FText::FromString( FString::Printf( TEXT( "%s%d" ), *Sign, amount ) ) );
		Text_Amount->SetColorAndOpacity( textColor );
	}
	if ( Img_ResourceIcon && icon )
		Img_ResourceIcon->SetBrushFromTexture( icon );
	if ( Text_Suffix )
	{
		Text_Suffix->SetText( FText::FromString( TEXT( "/ход  " ) ) );
		Text_Suffix->SetVisibility(
		    bShowTurnSuffix ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed
		);
	}
}

void UBuildingTooltipStatRow::Setup( UTexture2D* icon, const FString& statName, const FString& value )
{
	if ( Img_StatIcon && icon )
	{
		Img_StatIcon->SetBrushFromTexture( icon );
	}
	if ( Text_StatName )
	{
		Text_StatName->SetText( FText::FromString( statName ) );
	}
	if ( Text_Value )
	{
		Text_Value->SetText( FText::FromString( value ) );
	}	
}

void UBuildingTooltipHealthRow::Setup( UTexture2D* icon, const FString& healthValue )
{
	if ( Img_HealthIcon && icon )
	{
		Img_HealthIcon->SetBrushFromTexture( icon );
	}
	if ( Text_HealthValue )
	{
		Text_HealthValue->SetText( FText::FromString( healthValue ) );
	}
}

void UBuildingTooltipBonusRow::Setup(
    UTexture2D* targetIcon, UTexture2D* sourceIcon, float value, UTexture2D* resourceIcon
)
{
	if ( Img_TargetBuilding && targetIcon )
	{
		Img_TargetBuilding->SetBrushFromTexture( targetIcon );
	}
	if ( Img_SourceBuilding && sourceIcon )
	{
		Img_SourceBuilding->SetBrushFromTexture( sourceIcon );
	}	
	if ( Img_ResourceIcon && resourceIcon )
	{
		Img_ResourceIcon->SetBrushFromTexture( resourceIcon );
	}

	FString Sign = value > 0 ? TEXT( "+" ) : TEXT( "" );
	if ( Text_Value )
		Text_Value->SetText( FText::FromString( FString::Printf( TEXT( "%s%.0f" ), *Sign, value ) ) );
}

//logic main widgeth

void UBuildingTooltipWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	switch ( CurrentState )
	{
	case ETooltipState::DelayShow:
		StateTimer -= inDeltaTime;
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
		if ( WhiteFlash )
			WhiteFlash->SetRenderOpacity( FlashProgress );

		if ( FlashProgress <= 0.0f )
		{
			CurrentState = ETooltipState::Visible;
		}
		break;

	case ETooltipState::DelayHide:

		StateTimer -= inDeltaTime;
		if ( StateTimer <= 0.0f )
			CurrentState = ETooltipState::AnimatingOut;
		break;

	case ETooltipState::AnimatingOut:

		AnimProgress = FMath::Clamp( AnimProgress - ( inDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();

		if ( AnimProgress <= 0.0f )
			ForceHide();
		break;

	case ETooltipState::Visible:
	case ETooltipState::Hidden:
		break;
	}
}

void UBuildingTooltipWidget::ShowTooltip( TSubclassOf<ABuilding> buildingClass )
{
	if ( !buildingClass )
		return;

	PendingBuildingClass = buildingClass;

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
		{
			WhiteFlash->SetRenderOpacity( 1.0f );
		}

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
	{
		return;
	}
	float alpha = AnimationCurve ? AnimationCurve->GetFloatValue( AnimProgress ) : AnimProgress;
	AnimationContainer->SetRenderTranslation( FVector2D( FMath::Lerp( SlideOffsetX, 0.0f, alpha ), 0.0f ) );
	SetRenderOpacity( alpha );

	if ( CurrentState == ETooltipState::AnimatingIn || CurrentState == ETooltipState::AnimatingOut )
	{
		WhiteFlash->SetRenderOpacity( 1.0f );
	}
}

UTexture2D* UBuildingTooltipWidget::GetResourceIcon( EResourceType type )
{
	if ( UIConfig && UIConfig->ResourceIcons.Contains( type ) )
	{
		return UIConfig->ResourceIcons[type];
	}
	return nullptr;
}

UTexture2D* UBuildingTooltipWidget::GetStatIcon( EStatsType type )
{
	if ( UIConfig && UIConfig->StatIcons.Contains( type ) )
	{
		return UIConfig->StatIcons[type];
	}
	return nullptr;
}

void UBuildingTooltipWidget::UpdateContent()
{
	if (!CurrentBuildingClass)
	{
		return;
	}
		
	const ABuilding* cDO = CurrentBuildingClass->GetDefaultObject<ABuilding>();
	if ( !cDO )
	{
		return;
	}
		
	ClearContainers();
	UpdateHeader( cDO );
	UpdateEconomy( cDO );
	UpdateStats( cDO );
	UpdateBonuses();
}

void UBuildingTooltipWidget::ClearContainers()
{
	if ( Box_Cost )
	{
		Box_Cost->ClearChildren();
	}
	if ( Box_Maintenance )
	{
		Box_Maintenance->ClearChildren();
	}
	if ( Box_Production )
	{
		Box_Production->ClearChildren();
	}
	if ( Box_Stats )
	{
		Box_Stats->ClearChildren();
	}
	if ( Box_Bonus )
	{
		Box_Bonus->ClearChildren();
	}
	if ( Box_Health )
	{
		Box_Health->ClearChildren();
	}
}

void UBuildingTooltipWidget::UpdateHeader( const ABuilding* cDO )
{
	if ( UIConfig && UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		const FBuildingUIData& data = UIConfig->BuildingsData[CurrentBuildingClass];
		Text_Name->SetText( data.Name );
		Text_Description->SetText( data.Description );
		if ( Img_Icon )
		{
			Img_Icon->SetBrushFromTexture( data.Icon );
		}
	}
	else
	{
		Text_Name->SetText( FText::FromString( const_cast<ABuilding*>( cDO )->GetNameBuild() ) );
		Text_Description->SetText( FText::FromString( TEXT( "Нет описания" ) ) );
	}
}

void UBuildingTooltipWidget::UpdateEconomy( const ABuilding* cDO )
{
	APlayerController* PC = GetOwningPlayer();
	if ( !PC )
	{
		return;
	}

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

	auto AddResRow = [&]( UPanelWidget* Box, int32 Amount, EResourceType ResType, FSlateColor Color, bool bSuffix,
	                      bool bShowPlus = true )
	{
		if ( Amount != 0 && ResourceRowClass && Box )
		{
			UBuildingTooltipResourceRow* Row = CreateWidget<UBuildingTooltipResourceRow>( PC, ResourceRowClass );
			Row->Setup( Amount, GetResourceIcon( ResType ), Color, bSuffix, bShowPlus );
			Box->AddChild( Row );
		}
	};

	AddResRow( Box_Cost, costG, EResourceType::Gold, NeutralColor, false, false );
	AddResRow( Box_Cost, costF, EResourceType::Food, NeutralColor, false, false );
	AddResRow( Box_Cost, costP, EResourceType::Population, NeutralColor, false, false );

	AddResRow( Box_Maintenance, -maintG, EResourceType::Gold, ExpenseColor, true, false );
	AddResRow( Box_Maintenance, -maintF, EResourceType::Food, ExpenseColor, true, false );
	AddResRow( Box_Maintenance, -maintP, EResourceType::Population, ExpenseColor, false, false );

	AddResRow( Box_Production, prodG, EResourceType::Gold, IncomeColor, true );
	AddResRow( Box_Production, prodF, EResourceType::Food, IncomeColor, true );
	AddResRow( Box_Production, prodP, EResourceType::Population, IncomeColor, false );
}

void UBuildingTooltipWidget::UpdateStats( const ABuilding* cDO )
{
	APlayerController* PC = GetOwningPlayer();
	if ( !PC )
	{
		return;
	}

	FEntityStats stats = const_cast<ABuilding*>( cDO )->Stats();

	if ( HealthRowClass && Box_Health )
	{
		UBuildingTooltipHealthRow* HealthRow = CreateWidget<UBuildingTooltipHealthRow>( PC, HealthRowClass );
		HealthRow->Setup( GetStatIcon( EStatsType::MaxHealth ), FString::FromInt( stats.MaxHealth() ) );
		Box_Health->AddChild( HealthRow );
	}

	if ( cDO->IsA<ADefensiveBuilding>() )
	{
		auto AddStatRow = [&]( EStatsType Type, const FString& Name, const FString& Value )
		{
			if ( StatRowClass && Box_Stats )
			{
				UBuildingTooltipStatRow* Row = CreateWidget<UBuildingTooltipStatRow>( PC, StatRowClass );
				Row->Setup( GetStatIcon( Type ), Name, Value );
				Box_Stats->AddChild( Row );
			}
		};

		AddStatRow( EStatsType::AttackDamage, TEXT( "Урон" ), FString::FromInt( stats.AttackDamage() ) );
		AddStatRow(
		    EStatsType::AttackCooldown, TEXT( "Скорость" ), FString::Printf( TEXT( "%.1f" ), stats.AttackCooldown() )
		);
		AddStatRow( EStatsType::AttackRange, TEXT( "Радиус" ), FString::Printf( TEXT( "%.0f" ), stats.AttackRange() ) );
		AddStatRow(
		    EStatsType::SplashRadius, TEXT( "Область" ), stats.SplashRadius() > 0.0f ? TEXT( "Да" ) : TEXT( "Нет" )
		);
	}
}

void UBuildingTooltipWidget::UpdateBonuses()
{
	APlayerController* PC = GetOwningPlayer();
	if ( !PC )
	{
		return;
	}
		
	UTexture2D* BuildingIconTex = nullptr;
	if ( UIConfig && UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		BuildingIconTex = UIConfig->BuildingsData[CurrentBuildingClass].Icon;
	}

	UBuildingBonusComponent* bonusComp = UBuildingBonusComponent::FindInBlueprintClass( CurrentBuildingClass );
	if ( bonusComp && bonusComp->GetBonusEntries().Num() > 0 && BonusRowClass && Box_Bonus )
	{
		for ( const FBuildingBonusEntry& Entry : bonusComp->GetBonusEntries() )
		{
			UTexture2D* SourceIcon = nullptr;
			if ( Entry.SourceBuildingClass && UIConfig &&
			     UIConfig->BuildingsData.Contains( Entry.SourceBuildingClass ) )
			{
				SourceIcon = UIConfig->BuildingsData[Entry.SourceBuildingClass].Icon;
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