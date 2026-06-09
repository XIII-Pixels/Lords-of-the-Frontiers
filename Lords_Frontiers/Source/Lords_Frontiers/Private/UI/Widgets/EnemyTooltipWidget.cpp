#include "UI/Widgets/EnemyTooltipWidget.h"

#include "Lords_Frontiers/Public/Units/Unit.h"
#include "UI/InfoWaves/EnemyInfoDataAsset.h"
#include "UI/Widgets/BuildingUIConfig.h"
#include "Localization/GameLocalization.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

void UEnemyTooltipWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	const bool bIsVisibleState =
	    ( CurrentState == ETooltipState::Visible ) || ( CurrentState == ETooltipState::AnimatingIn ) ||
	    ( CurrentState == ETooltipState::HoldFlash ) || ( CurrentState == ETooltipState::FadeFlash );

	if ( bIsVisibleState )
	{
		AUnit* currentEnemy = CurrentEnemy_.Get();

		if ( !IsValid( currentEnemy ) || !currentEnemy->Stats().IsAlive() || currentEnemy->IsActorBeingDestroyed() )
		{
			HideTooltip();
		}
		else
		{
			constexpr float kRefreshInterval = 0.1f;
			InstanceRefreshAccumulator_ += inDeltaTime;
			if ( InstanceRefreshAccumulator_ >= kRefreshInterval )
			{
				InstanceRefreshAccumulator_ = 0.0f;
				RefreshFromInstance();
			}
		}
	}

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
	case ETooltipState::Hidden:
		break;
	}
}

void UEnemyTooltipWidget::ShowTooltip( AUnit* enemyUnit )
{
	if ( !IsValid( enemyUnit ) || !enemyUnit->Stats().IsAlive() )
	{
		ForceHide();
		return;
	}

	PendingEnemy_ = enemyUnit;
	SetVisibility( ESlateVisibility::HitTestInvisible );

	if ( ( CurrentState == ETooltipState::Hidden ) || ( CurrentState == ETooltipState::AnimatingOut ) ||
	     ( CurrentState == ETooltipState::DelayHide ) )
	{
		CurrentState = ETooltipState::DelayShow;
		StateTimer = ShowDelay;
		AnimProgress = 0.0f;
		SetRenderOpacity( 0.0f );

		CurrentEnemy_ = PendingEnemy_;
		UpdateContent();

		SpawnRangeIndicator( CurrentEnemy_.Get() );
	}
	else if ( ( CurrentState == ETooltipState::Visible ) || ( CurrentState == ETooltipState::FadeFlash ) ||
	          ( CurrentState == ETooltipState::HoldFlash ) )
	{
		if ( IsValid( WhiteFlash ) )
		{
			WhiteFlash->SetRenderOpacity( 1.0f );
		}

		CurrentEnemy_ = PendingEnemy_;
		UpdateContent();

		SpawnRangeIndicator( CurrentEnemy_.Get() );

		CurrentState = ETooltipState::HoldFlash;
		StateTimer = SwitchDelay;
		AnimProgress = 1.0f;
	}
}

void UEnemyTooltipWidget::HideTooltip()
{
	if ( ( CurrentState != ETooltipState::Hidden ) && ( CurrentState != ETooltipState::AnimatingOut ) &&
	     ( CurrentState != ETooltipState::DelayHide ) )
	{
		CurrentState = ETooltipState::DelayHide;
		StateTimer = HideDelay;
	}
}

void UEnemyTooltipWidget::ForceHide()
{
	ClearRangeIndicator();

	SetRenderOpacity( 0.0f );
	CurrentState = ETooltipState::Hidden;
	StateTimer = 0.0f;
	AnimProgress = 0.0f;
	FlashProgress = 0.0f;
	CurrentEnemy_ = nullptr;
	InstanceRefreshAccumulator_ = 0.0f;
	LastSnapshot_ = FEnemySnapshot{};

	SetVisibility( ESlateVisibility::Hidden );
}

void UEnemyTooltipWidget::ApplyAnimation()
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

UTexture2D* UEnemyTooltipWidget::GetStatIcon( EStatsType type ) const
{
	if ( IsValid( GlobalUIConfig ) && GlobalUIConfig->StatIcons.Contains( type ) )
	{
		return GlobalUIConfig->StatIcons[type];
	}
	return nullptr;
}

void UEnemyTooltipWidget::UpdateContent()
{
	AUnit* enemy = CurrentEnemy_.Get();
	if ( !IsValid( enemy ) )
	{
		return;
	}

	TSubclassOf<AUnit> enemyClass = enemy->GetClass();
	if ( IsValid( EnemyDataAsset ) && EnemyDataAsset->EnemyDataMap.Contains( enemyClass ) )
	{
		const FEnemyUIData& data = EnemyDataAsset->EnemyDataMap[enemyClass];
		if ( IsValid( Text_Name ) )
			Text_Name->SetText( data.EnemyName );
		if ( IsValid( Text_Description ) )
			Text_Description->SetText( data.EnemyDescription );
		if ( IsValid( Img_Icon ) )
			Img_Icon->SetBrushFromTexture( data.EnemyIcon );
	}

	LastSnapshot_ = CaptureSnapshot( enemy );

	APlayerController* pc = GetOwningPlayer();
	if ( !IsValid( pc ) )
		return;

	if ( IsValid( HealthRowClass ) && IsValid( Box_Health ) )
	{
		if ( !IsValid( CachedHealthRow ) )
		{
			CachedHealthRow = CreateWidget<UBuildingTooltipHealthRow>( pc, HealthRowClass );
			Box_Health->AddChild( CachedHealthRow );
		}

		if ( IsValid( CachedHealthRow ) )
		{
			CachedHealthRow->Setup( GetStatIcon( EStatsType::MaxHealth ), FString::FromInt( LastSnapshot_.Health ) );
		}
	}

	if ( IsValid( StatRowClass ) && IsValid( Box_Stats ) )
	{
		auto UpdateOrAddStatRow = [&]( TObjectPtr<UBuildingTooltipStatRow>& RowCache, EStatsType type,
		                               const FString& name, const FString& value )
		{
			if ( !IsValid( RowCache ) )
			{
				RowCache = CreateWidget<UBuildingTooltipStatRow>( pc, StatRowClass );
				Box_Stats->AddChild( RowCache );
			}

			if ( IsValid( RowCache ) )
			{
				RowCache->Setup( GetStatIcon( type ), name, value );
			}
		};

		UpdateOrAddStatRow(
		    CachedDamageRow, EStatsType::AttackDamage, TEXT( "Damage" ), FString::FromInt( LastSnapshot_.AttackDamage )
		);

		UpdateOrAddStatRow(
		    CachedSpeedRow, EStatsType::MaxSpeed, TEXT( "Speed" ),
		    FString::FromInt( FMath::RoundToInt( LastSnapshot_.MaxSpeed ) )
		);

		UpdateOrAddStatRow(
		    CachedRangeRow, EStatsType::AttackRange, TEXT( "Radius" ),
		    FString::FromInt( FMath::RoundToInt( LastSnapshot_.AttackRange ) )
		);

		UpdateOrAddStatRow(
		    CachedCooldownRow, EStatsType::AttackCooldown, TEXT( "Interval" ),
		    FString::Printf( TEXT( "%.1f" ), LastSnapshot_.AttackCooldown )
		);

		UpdateOrAddStatRow(
		    CachedCritChanceRow, EStatsType::CritChance, LF_LOC( "Stats.CritChance" ).ToString(),
		    FString::Printf( TEXT( "%d%%" ), LastSnapshot_.CritChance )
		);

		UpdateOrAddStatRow(
		    CachedCritDamageRow, EStatsType::CritDamage, LF_LOC( "Stats.CritDamage" ).ToString(),
		    FString::Printf( TEXT( "+%d%%" ), LastSnapshot_.CritDamageBonus )
		);
	}
}

void UEnemyTooltipWidget::RefreshFromInstance()
{
	AUnit* enemy = CurrentEnemy_.Get();
	if ( !IsValid( enemy ) )
		return;

	const FEnemySnapshot currentSnapshot = CaptureSnapshot( enemy );
	if ( LastSnapshot_.Equals( currentSnapshot ) )
	{
		return;
	}

	UpdateContent();
}

bool UEnemyTooltipWidget::FEnemySnapshot::Equals( const FEnemySnapshot& other ) const
{
	return ( bValid == other.bValid ) && ( Health == other.Health ) && ( MaxHealth == other.MaxHealth ) &&
	       ( AttackDamage == other.AttackDamage ) && FMath::IsNearlyEqual( AttackRange, other.AttackRange ) &&
	       FMath::IsNearlyEqual( AttackCooldown, other.AttackCooldown ) &&
	       FMath::IsNearlyEqual( MaxSpeed, other.MaxSpeed ) && ( CritChance == other.CritChance ) &&
	       ( CritDamageBonus == other.CritDamageBonus );
}

UEnemyTooltipWidget::FEnemySnapshot UEnemyTooltipWidget::CaptureSnapshot( const AUnit* enemy ) const
{
	FEnemySnapshot snapshot;
	if ( !IsValid( enemy ) )
		return snapshot;

	snapshot.bValid = true;
	const FEntityStats& stats = enemy->Stats();

	snapshot.Health = stats.Health();
	snapshot.MaxHealth = stats.MaxHealth();
	snapshot.AttackDamage = stats.AttackDamage();
	snapshot.AttackRange = stats.AttackRange();
	snapshot.AttackCooldown = stats.AttackCooldown();
	snapshot.MaxSpeed = stats.MaxSpeed();
	snapshot.CritChance = stats.CritChance();
	snapshot.CritDamageBonus = stats.CritDamageBonus();

	return snapshot;
}

void UEnemyTooltipWidget::SpawnRangeIndicator( AUnit* enemy )
{
	ClearRangeIndicator();

	if ( !IsValid( enemy ) || !IsValid( RangeIndicatorMaterial ) )
	{
		return;
	}

	float attackRange = enemy->Stats().AttackRange();

	FVector DecalSize = FVector( 3000.0f, attackRange, attackRange );

	SpawnedRangeDecal = UGameplayStatics::SpawnDecalAttached(
	    RangeIndicatorMaterial, DecalSize, enemy->GetRootComponent(), NAME_None, FVector::ZeroVector,
	    FRotator( -90.0f, 0.0f, 0.0f ), EAttachLocation::KeepRelativeOffset, 0.0f
	);

	if ( IsValid( SpawnedRangeDecal ) )
	{

		SpawnedRangeDecal->SetAbsolute( false, false, true );

		SpawnedRangeDecal->SetWorldScale3D( FVector( 1.0f, 1.0f, 1.0f ) );
	}
}

void UEnemyTooltipWidget::ClearRangeIndicator()
{
	if ( IsValid( SpawnedRangeDecal ) )
	{
		SpawnedRangeDecal->DestroyComponent();
		SpawnedRangeDecal = nullptr;
	}
}