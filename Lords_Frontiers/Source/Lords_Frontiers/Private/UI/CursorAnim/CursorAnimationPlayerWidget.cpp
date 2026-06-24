#include "UI/CursorAnim/CursorAnimationPlayerWidget.h"

#include "UI/CursorAnim/CursorAnimationConfig.h"

#include "Components/Image.h"

void UCursorAnimationPlayerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetVisibility( ESlateVisibility::HitTestInvisible );
}

void UCursorAnimationPlayerWidget::Play( const FCursorAnimationEntry& entry )
{
	SetVisibility( ESlateVisibility::HitTestInvisible );

	Frames_ = entry.Frames;
	Fps_ = FMath::Max( 0.01f, entry.FramesPerSecond );
	Elapsed_ = 0.0f;
	CurrentFrame_ = -1;
	bPlaying_ = Frames_.Num() > 0;

	if ( AnimImage )
	{
		AnimImage->SetDesiredSizeOverride( entry.DisplaySize );
		if ( bPlaying_ && Frames_[ 0 ] )
		{
			AnimImage->SetBrushFromTexture( Frames_[ 0 ], false );
			CurrentFrame_ = 0;
		}
	}

	SetDesiredSizeInViewport( entry.DisplaySize );
}

void UCursorAnimationPlayerWidget::NativeTick( const FGeometry& myGeometry, float deltaTime )
{
	Super::NativeTick( myGeometry, deltaTime );

	if ( !bPlaying_ )
	{
		return;
	}

	Elapsed_ += deltaTime;
	const int32 frame = FMath::FloorToInt( Elapsed_ * Fps_ );

	if ( frame >= Frames_.Num() )
	{
		bPlaying_ = false;
		OnFinished.Broadcast( this );
		return;
	}

	if ( frame != CurrentFrame_ && AnimImage )
	{
		CurrentFrame_ = frame;
		if ( Frames_[ frame ] )
		{
			AnimImage->SetBrushFromTexture( Frames_[ frame ], false );
		}
	}
}
