#include "Tutorial/TutorialBubbleWidget.h"

#include "Components/Image.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Layout/Geometry.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Tutorial/TutorialSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC( LogTutorialBubble, Log, All );

void UTutorialBubbleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if ( DimImage )
	{
		DimMID_ = DimImage->GetDynamicMaterial();
	}
	else
	{
		UE_LOG( LogTutorialBubble, Warning, TEXT( "DimImage не привязан в %s" ), *GetName() );
	}
}

void UTutorialBubbleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().SetTimerForNextTick( this, &UTutorialBubbleWidget::SyncHoles );
	}
}

void UTutorialBubbleWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	if ( !DimImage )
	{
		return;
	}

	const FVector2D currentSize = DimImage->GetCachedGeometry().GetAbsoluteSize();
	if ( !currentSize.Equals( LastDimSize_, 0.5f ) )
	{
		LastDimSize_ = currentSize;
		SyncHoles();
	}

	UpdateDimHitTestability();
}

FReply UTutorialBubbleWidget::NativeOnMouseButtonDown( const FGeometry& geometry, const FPointerEvent& mouseEvent )
{
	if ( mouseEvent.GetEffectingButton() != EKeys::LeftMouseButton )
	{
		return Super::NativeOnMouseButtonDown( geometry, mouseEvent );
	}

	UTutorialSubsystem* sub = UTutorialSubsystem::Get( this );
	if ( !sub || sub->GetCurrentStepAdvance() != ETutorialAdvance::ClickAnywhere )
	{
		return Super::NativeOnMouseButtonDown( geometry, mouseEvent );
	}

	sub->OnBubbleClickAnywhere();
	return FReply::Handled();
}

void UTutorialBubbleWidget::UpdateDimHitTestability()
{
	if ( !DimImage )
	{
		return;
	}

	ETutorialAdvance advance = ETutorialAdvance::None;
	if ( const UTutorialSubsystem* sub = UTutorialSubsystem::Get( this ) )
	{
		advance = sub->GetCurrentStepAdvance();
	}

	const ESlateVisibility desired = ( advance == ETutorialAdvance::ClickAnywhere )
	    ? ESlateVisibility::Visible
	    : ESlateVisibility::HitTestInvisible;

	if ( DimImage->GetVisibility() != desired )
	{
		DimImage->SetVisibility( desired );
	}
}

void UTutorialBubbleWidget::SyncHoles()
{
	if ( !DimMID_ )
	{
		return;
	}

	auto pushHole = [ this ]( const UWidget* zone, FName paramName )
	{
		if ( zone && zone->IsVisible() )
		{
			DimMID_->SetVectorParameterValue( paramName, ComputeHoleRectUV( zone ) );
		}
		else
		{
			DimMID_->SetVectorParameterValue( paramName, FLinearColor::Transparent );
		}
	};

	pushHole( HoleZone1, TEXT( "HoleRect1" ) );
	pushHole( HoleZone2, TEXT( "HoleRect2" ) );
}

bool UTutorialBubbleWidget::IsPointInWidget( const UWidget* widget, const FVector2D& screenPosAbs ) const
{
	if ( !widget || !widget->IsVisible() )
	{
		return false;
	}
	const FGeometry& geo = widget->GetCachedGeometry();
	const FVector2D topLeft = geo.LocalToAbsolute( FVector2D::ZeroVector );
	const FVector2D size = geo.GetAbsoluteSize();
	return screenPosAbs.X >= topLeft.X && screenPosAbs.X < topLeft.X + size.X
	       && screenPosAbs.Y >= topLeft.Y && screenPosAbs.Y < topLeft.Y + size.Y;
}

FLinearColor UTutorialBubbleWidget::ComputeHoleRectUV( const UWidget* targetWidget ) const
{
	if ( !targetWidget || !DimImage )
	{
		return FLinearColor::Transparent;
	}

	const FGeometry& targetGeo = targetWidget->GetCachedGeometry();
	const FGeometry& dimGeo = DimImage->GetCachedGeometry();

	const FVector2D targetAbsPos = targetGeo.LocalToAbsolute( FVector2D::ZeroVector );
	const FVector2D targetAbsSize = targetGeo.GetAbsoluteSize();
	const FVector2D dimAbsPos = dimGeo.LocalToAbsolute( FVector2D::ZeroVector );
	const FVector2D dimAbsSize = dimGeo.GetAbsoluteSize();

	if ( dimAbsSize.X <= 0.f || dimAbsSize.Y <= 0.f )
	{
		return FLinearColor::Transparent;
	}

	const FVector2D halfAbs = targetAbsSize * 0.5f;
	const FVector2D centerUV = ( ( targetAbsPos + halfAbs ) - dimAbsPos ) / dimAbsSize;
	const FVector2D halfUV = halfAbs / dimAbsSize;

	return FLinearColor( centerUV.X, centerUV.Y, halfUV.X, halfUV.Y );
}
