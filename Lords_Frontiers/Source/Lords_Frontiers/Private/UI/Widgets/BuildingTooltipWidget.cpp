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
			ForceHide();
		break;

	case ETooltipState::Visible:
		break;

	case ETooltipState::Hidden:
		return;
	}
}

void UBuildingTooltipWidget::ShowTooltip( TSubclassOf<ABuilding> BuildingClass )
{
	if ( GEngine )
		GEngine->AddOnScreenDebugMessage( -1, 2.f, FColor::Yellow, TEXT( "Hover detected!" ) );

	if ( !BuildingClass )
		return;

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
		return;

	float Alpha = AnimationCurve ? AnimationCurve->GetFloatValue( AnimProgress ) : AnimProgress;

	float CurrentX = FMath::Lerp( SlideOffsetX, 0.0f, Alpha );
	AnimationContainer->SetRenderTranslation( FVector2D( CurrentX, 0.0f ) );

	SetRenderOpacity( Alpha );

	WhiteFlash->SetRenderOpacity( 1.0f - Alpha );
}

void UBuildingTooltipWidget::UpdateContent()
{
	if ( !CurrentBuildingClass )
		return;
	const ABuilding* CDO = CurrentBuildingClass->GetDefaultObject<ABuilding>();
	if ( !CDO )
		return;

	if ( UIConfig && UIConfig->BuildingsData.Contains( CurrentBuildingClass ) )
	{
		const FBuildingUIData& Data = UIConfig->BuildingsData[CurrentBuildingClass];
		Text_Name->SetText( Data.Name );
		Text_Description->SetText( Data.Description );
		if ( Img_Icon )
			Img_Icon->SetBrushFromTexture( Data.Icon );
	}
	else
	{
		FString BuildingName = const_cast<ABuilding*>( CDO )->GetNameBuild();
		Text_Name->SetText( FText::FromString( BuildingName ) );
		Text_Description->SetText( FText::FromString( TEXT( "No description in config" ) ) );
	}

	FEntityStats Stats = const_cast<ABuilding*>( CDO )->Stats();
	FString StatsStr = FString::Printf( TEXT( "Strength: %d\n" ), Stats.MaxHealth() );

	if ( CDO->IsA<ADefensiveBuilding>() )
	{
		StatsStr += FString::Printf(
		    TEXT( "Damage: %d\nSpeed damage: %.1f\nRadius: %.0f\nArea damage: %s" ), Stats.AttackDamage(),
		    Stats.AttackCooldown(), Stats.AttackRange(), Stats.SplashRadius() > 0.0f ? TEXT( "Yes" ) : TEXT( "No" )
		);
	}
	Text_Stats->SetText( FText::FromString( StatsStr ) );

	int32 CostG = CDO->GetBuildingCost().Gold;
	int32 CostF = CDO->GetBuildingCost().Food;
	int32 CostP = CDO->GetBuildingCost().Population;

	int32 MaintG = CDO->GetMaintenanceCost().Gold;
	int32 MaintF = CDO->GetMaintenanceCost().Food;
	int32 MaintP = CDO->GetMaintenanceCost().Population;

	int32 ProdG = 0, ProdF = 0, ProdP = 0;
	if ( const AResourceBuilding* ResB = Cast<AResourceBuilding>( CDO ) )
	{
		ProdG = ResB->GetProductionConfig().Gold;
		ProdF = ResB->GetProductionConfig().Food;
		ProdP = ResB->GetProductionConfig().Population;
	}

	if ( CostP < 0 )
	{
		ProdP += FMath::Abs( CostP );
		CostP = 0;
	}
	if ( MaintP < 0 )
	{
		ProdP += FMath::Abs( MaintP );
		MaintP = 0;
	}

	Text_Cost->SetText( FText::FromString( FormatResourceString( CostG, CostF, CostP, 0 ) ) );
	Text_Maintenance->SetText( FText::FromString( FormatResourceString( MaintG, MaintF, MaintP, 0 ) ) );
	Text_Production->SetText( FText::FromString( FormatResourceString( ProdG, ProdF, ProdP, 0 ) ) );

	UBuildingBonusComponent* BonusComp = UBuildingBonusComponent::FindInBlueprintClass( CurrentBuildingClass );
	if ( BonusComp && BonusComp->GetBonusEntries().Num() > 0 )
	{
		FString BonusStr = TEXT( "" );
		for ( const FBuildingBonusEntry& Entry : BonusComp->GetBonusEntries() )
		{
			FString SourceName = TEXT( "Build" );
			if ( Entry.SourceBuildingClass )
			{
				ABuilding* SourceCDO =
				    const_cast<ABuilding*>( Entry.SourceBuildingClass->GetDefaultObject<ABuilding>() );
				if ( SourceCDO )
				{
					SourceName = SourceCDO->GetNameBuild();
				}
			}
			BonusStr += FString::Printf( TEXT( "Next to [%s] gives +%.0f\n" ), *SourceName, Entry.Value );
		}
		Text_Bonus->SetText( FText::FromString( BonusStr ) );
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