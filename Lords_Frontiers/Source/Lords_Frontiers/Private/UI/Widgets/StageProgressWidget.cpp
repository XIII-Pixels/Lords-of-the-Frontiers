#include "UI/Widgets/StageProgressWidget.h"

void UStageProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyBarTranslation( 0.0f );
	InitFlags();
}

void UStageProgressWidget::InitFlags()
{
	FlagStates_.Empty();

	auto AddFlag = [this]( UImage* img, float threshold )
	{
		if ( img )
		{
			FFlagAnimState state;
			state.Image = img;
			state.Threshold = threshold;
			state.CurrentOffset = 0.0f;
			state.bRaised = false;
			FlagStates_.Add( state );
			ApplyFlagTranslation( FlagStates_.Last() );
		}
	};

	AddFlag( FlagStart, FlagStartThreshold );
	AddFlag( FlagMiddle, FlagMiddleThreshold );
	AddFlag( FlagEnd, FlagEndThreshold );
}

void UStageProgressWidget::SetTargetProgress( float newTarget )
{
	newTarget = FMath::Clamp( newTarget, 0.0f, 1.0f );

	if ( FMath::IsNearlyEqual( newTarget, TargetProgress_ ) )
	{
		return;
	}

	StartProgress_ = CurrentProgress_;
	TargetProgress_ = newTarget;
	Elapsed_ = 0.0f;
	bAnimating_ = true;
}

void UStageProgressWidget::ResetProgress()
{
	SetTargetProgress( 0.0f );
}

void UStageProgressWidget::ResetProgressImmediate()
{
	bAnimating_ = false;
	CurrentProgress_ = 0.0f;
	TargetProgress_ = 0.0f;
	StartProgress_ = 0.0f;
	Elapsed_ = 0.0f;
	ApplyBarTranslation( 0.0f );
}

void UStageProgressWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	if ( bAnimating_ )
	{
		bool bGoingRight = ( TargetProgress_ > StartProgress_ );
		float duration = bGoingRight ? FMath::Max( FillDuration, 0.01f ) : FMath::Max( ResetDuration, 0.01f );

		Elapsed_ += inDeltaTime;
		float alpha = FMath::Clamp( Elapsed_ / duration, 0.0f, 1.0f );

		float progress = FMath::Lerp( StartProgress_, TargetProgress_, alpha );
		CurrentProgress_ = progress;
		ApplyBarTranslation( progress );

		if ( alpha >= 1.0f )
		{
			bAnimating_ = false;
			CurrentProgress_ = TargetProgress_;
			ApplyBarTranslation( TargetProgress_ );
		}
	}

	TickFlags( inDeltaTime );
}

void UStageProgressWidget::ApplyBarTranslation( float progress )
{
	if ( !ClipPanel )
	{
		return;
	}

	float width = FMath::Lerp( ClosedWidth, PanelWidth, progress );

	if ( width <= 0.0f )
	{
		ClipPanel->SetVisibility( ESlateVisibility::Collapsed );
	}
	else
	{
		ClipPanel->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
		ClipPanel->SetWidthOverride( width );
	}
}

void UStageProgressWidget::TickFlags( float deltaTime )
{
	float animSpeed = ( FlagAnimDuration > 0.0f ) ? ( FlagRiseOffset / FlagAnimDuration ) : FlagRiseOffset * 100.0f;

	for ( FFlagAnimState& flag : FlagStates_ )
	{
		bool bShouldBeRaised = ( CurrentProgress_ >= flag.Threshold );

		if ( bShouldBeRaised && !flag.bRaised )
		{
			flag.bRaised = true;
		}
		else if ( !bShouldBeRaised && flag.bRaised )
		{
			flag.bRaised = false;
		}

		float target = flag.bRaised ? FlagRiseOffset : 0.0f;

		if ( !FMath::IsNearlyEqual( flag.CurrentOffset, target, 0.1f ) )
		{
			float direction = ( target > flag.CurrentOffset ) ? 1.0f : -1.0f;
			flag.CurrentOffset += direction * animSpeed * deltaTime;
			flag.CurrentOffset = ( direction > 0.0f ) ? FMath::Min( flag.CurrentOffset, target )
			                                          : FMath::Max( flag.CurrentOffset, target );

			ApplyFlagTranslation( flag );
		}
		else if ( !FMath::IsNearlyEqual( flag.CurrentOffset, target ) )
		{
			flag.CurrentOffset = target;
			ApplyFlagTranslation( flag );
		}
	}
}

void UStageProgressWidget::ApplyFlagTranslation( FFlagAnimState& flag )
{
	if ( flag.Image )
	{
		flag.Image->SetRenderTranslation( FVector2D( 0.0f, -flag.CurrentOffset ) );
	}
}