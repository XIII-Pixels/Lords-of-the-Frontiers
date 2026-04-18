#include "UI/HealthBar/HealthBarWidget.h"

#include "Components/ProgressBar.h"

void UHealthBarWidget::SetHealth( int newHealth, int maxHealth )
{
	const float safeMax = FMath::Max( 1, maxHealth );
	const float newTarget = FMath::Clamp( static_cast<float>( newHealth ) / safeMax, 0.0f, 1.0f );

	if ( !FMath::IsNearlyEqual( newTarget, TargetPercent_ ) )
	{
		DrainStartPercent_ = DisplayedPercent_;
		TargetPercent_ = newTarget;
		DrainElapsed_ = 0.0f;
	}
}

void UHealthBarWidget::SnapToTarget()
{
	DisplayedPercent_ = TargetPercent_;
	DrainStartPercent_ = TargetPercent_;
	DrainElapsed_ = 0.0f;

	if ( HealthBar )
	{
		HealthBar->SetPercent( DisplayedPercent_ );
	}
}

void UHealthBarWidget::TickAnim( float deltaTime )
{
	if ( FMath::IsNearlyEqual( DisplayedPercent_, TargetPercent_ ) )
	{
		DisplayedPercent_ = TargetPercent_;
	}
	else
	{
		const FRichCurve* curve = DrainCurve_.GetRichCurveConst();
		const bool bHasCurve = curve && curve->GetNumKeys() > 0;

		if ( bHasCurve )
		{
			DrainElapsed_ += deltaTime;
			const float alpha = FMath::Clamp( curve->Eval( DrainElapsed_ ), 0.0f, 1.0f );
			DisplayedPercent_ = FMath::Lerp( DrainStartPercent_, TargetPercent_, alpha );
		}
		else
		{
			DisplayedPercent_ = FMath::FInterpTo( DisplayedPercent_, TargetPercent_, deltaTime, LerpSpeed_ );
		}
	}

	if ( HealthBar )
	{
		HealthBar->SetPercent( DisplayedPercent_ );
	}
}

void UHealthBarWidget::ApplyCameraScale( float zoomAlpha )
{
	const float clamped = FMath::Clamp( zoomAlpha, 0.0f, 1.0f );
	const float scale = FMath::Lerp( MaxScale_, MinScale_, clamped );

	UE_LOG(
	    LogTemp, Warning, TEXT( "[HPBar] widget %s alpha=%.3f Min=%.2f Max=%.2f -> scale=%.3f" ), *GetName(), clamped,
	    MinScale_, MaxScale_, scale
	);

	FWidgetTransform transform = GetRenderTransform();
	transform.Scale = FVector2D( scale, scale );
	SetRenderTransform( transform );
}
