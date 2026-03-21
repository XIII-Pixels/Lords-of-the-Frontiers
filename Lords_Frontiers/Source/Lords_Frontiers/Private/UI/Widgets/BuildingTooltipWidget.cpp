#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Building/Bonus/BuildingBonusComponent.h"
#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/ResourceBuilding.h"
#include "UI/Widgets/BuildingUIConfig.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UBuildingTooltipWidget::NativeTick( const FGeometry& MyGeometry, float InDeltaTime )
{
	Super::NativeTick( MyGeometry, InDeltaTime );

	switch ( CurrentState )
	{
	case ETooltipState::DelayShow:
		StateTimer -= InDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			CurrentState = ETooltipState::AnimatingIn;
			SetVisibility( ESlateVisibility::HitTestInvisible );
		}
		break;

	case ETooltipState::DelayHide:
		StateTimer -= InDeltaTime;
		if ( StateTimer <= 0.0f )
		{
			CurrentState = ETooltipState::AnimatingOut;
		}
		break;

	case ETooltipState::AnimatingIn:
		AnimProgress = FMath::Clamp( AnimProgress + ( InDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();
		if ( AnimProgress >= 1.0f )
		{
			CurrentState = ETooltipState::Visible;
			SetRenderOpacity( 1.0f );
		}
		break;

	case ETooltipState::AnimatingOut:
		AnimProgress = FMath::Clamp( AnimProgress - ( InDeltaTime / AnimDuration ), 0.0f, 1.0f );
		ApplyAnimation();
		if ( AnimProgress <= 0.0f )
		{
			ForceHide();
		}
		break;

	case ETooltipState::Visible:
		break;

	case ETooltipState::Hidden:
		return;
	}
}

void UBuildingTooltipWidget::ShowTooltip( TSubclassOf<ABuilding> BuildingClass )
{
	if ( !BuildingClass )
	{
		return;
	}

	CurrentBuildingClass = BuildingClass;
	UpdateContent();

	if ( CurrentState == ETooltipState::Hidden || CurrentState == ETooltipState::DelayHide ||
	     CurrentState == ETooltipState::AnimatingOut )
	{
		CurrentState = ETooltipState::DelayShow;
		StateTimer = ShowDelay;

		SetVisibility( ESlateVisibility::HitTestInvisible );
		ApplyAnimation(); 
	}
}

void UBuildingTooltipWidget::HideTooltip()
{
	if ( CurrentState == ETooltipState::Visible || CurrentState == ETooltipState::AnimatingIn ||
	     CurrentState == ETooltipState::DelayShow )
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

	float CurrentX = FMath::Lerp( SlideOffsetX, 0.0f, alpha );
	AnimationContainer->SetRenderTranslation( FVector2D( CurrentX, 0.0f ) );

	SetRenderOpacity( alpha );

	WhiteFlash->SetRenderOpacity( 1.0f - alpha );
}

void UBuildingTooltipWidget::UpdateContent()
{
	if ( !CurrentBuildingClass )
	{
		return;
	}
	const ABuilding* cDO = CurrentBuildingClass->GetDefaultObject<ABuilding>();
	if ( !cDO )
	{
		return;
	}

	if ( UIConfig && UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		const FBuildingUIData& data = UIConfig->BuildingsData[CurrentBuildingClass];
		Text_Name->SetText( data.Name );
		Text_Description->SetText( data.Description );
		if ( Img_Icon )
			Img_Icon->SetBrushFromTexture( data.Icon );
	}
	else
	{
		FString buildingName = const_cast<ABuilding*>( cDO )->GetNameBuild();
		Text_Name->SetText( FText::FromString( buildingName ) );
		Text_Description->SetText( FText::FromString( TEXT( "No description in config" ) ) );
	}

	FEntityStats stats = const_cast<ABuilding*>( cDO )->Stats();
	FString statsStr = FString::Printf( TEXT( "Strength: %d\n" ), stats.MaxHealth() );

	if ( cDO->IsA<ADefensiveBuilding>() )
	{
		statsStr += FString::Printf(
		    TEXT( "Damage: %d\nSpeed damage: %.1f\nRadius: %.0f\nArea damage: %s" ), stats.AttackDamage(),
		    stats.AttackCooldown(), stats.AttackRange(), stats.SplashRadius() > 0.0f ? TEXT( "Yes" ) : TEXT( "No" )
		);
	}
	Text_Stats->SetText( FText::FromString( statsStr ) );

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

	Text_Cost->SetText( FText::FromString( FormatResourceString( costG, costF, costP, 0 ) ) );
	Text_Maintenance->SetText( FText::FromString( FormatResourceString( maintG, maintF, maintP, 0 ) ) );
	Text_Production->SetText( FText::FromString( FormatResourceString( prodG, prodF, prodP, 0 ) ) );

	UBuildingBonusComponent* bonusComp = UBuildingBonusComponent::FindInBlueprintClass( CurrentBuildingClass );
	if ( bonusComp && bonusComp->GetBonusEntries().Num() > 0 )
	{
		FString bonusStr = TEXT( "" );
		for ( const FBuildingBonusEntry& Entry : bonusComp->GetBonusEntries() )
		{
			FString sourceName = TEXT( "Build" );
			if ( Entry.SourceBuildingClass )
			{
				ABuilding* sourceCDO =
				    const_cast<ABuilding*>( Entry.SourceBuildingClass->GetDefaultObject<ABuilding>() );
				if ( sourceCDO )
				{
					sourceName = sourceCDO->GetNameBuild();
				}
			}
			bonusStr += FString::Printf( TEXT( "Next to [%s] gives +%.0f\n" ), *sourceName, Entry.Value );
		}
		Text_Bonus->SetText( FText::FromString( bonusStr ) );
	}
	else
	{
		Text_Bonus->SetText( FText::FromString( TEXT( "No bonus" ) ) );
	}
}

FString UBuildingTooltipWidget::FormatResourceString( int32 Gold, int32 Food, int32 Pop, int32 Prog )
{
	TArray<FString> Parts;
	if ( Gold > 0 )
		Parts.Add( FString::Printf( TEXT( "%d Gold" ), Gold ) );
	if ( Food > 0 )
		Parts.Add( FString::Printf( TEXT( "%d Food" ), Food ) );
	if ( Pop > 0 )
		Parts.Add( FString::Printf( TEXT( "%d Population" ), Pop ) );
	if ( Prog > 0 )
		Parts.Add( FString::Printf( TEXT( "%d Progress" ), Prog ) );

	return Parts.Num() > 0 ? FString::Join( Parts, TEXT( ", " ) ) : TEXT( "-" );
}