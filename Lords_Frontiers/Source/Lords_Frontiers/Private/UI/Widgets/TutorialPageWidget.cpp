#include "UI/Widgets/TutorialPageWidget.h"

#include "TimerManager.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UTutorialPageWidget::SetData( const FTutorialPageData& inData )
{
	StopPlayback();

	CurrentData_ = inData;
	CurrentFrames_ = inData.Frames;
	CurrentFrameIndex_ = 0;
	CurrentFPS_ = FMath::Max( inData.FPS, 1.0f );

	ApplyStaticContent();
}

void UTutorialPageWidget::OnPageActivated()
{
	bPlaying_ = true;

	if ( CurrentFrames_.Num() > 0 )
	{
		StartPlayback();
	}
	else
	{
		ApplyStaticContent();
	}
}

void UTutorialPageWidget::OnPageDeactivated()
{
	bPlaying_ = false;
	StopPlayback();
}

void UTutorialPageWidget::NativeDestruct()
{
	StopPlayback();
	Super::NativeDestruct();
}

void UTutorialPageWidget::ApplyStaticContent()
{
	if ( TitleText_ )
	{
		TitleText_->SetText( CurrentData_.Title );
	}

	if ( TutorialText_ )
	{
		TutorialText_->SetText( CurrentData_.Body );
	}

	if ( !TutorialImage_ )
	{
		return;
	}

	if ( CurrentFrames_.Num() > 0 && CurrentFrames_[0] )
	{
		TutorialImage_->SetBrushFromTexture( CurrentFrames_[0], true );
	}
	else if ( CurrentData_.Image )
	{
		TutorialImage_->SetBrushFromTexture( CurrentData_.Image, true );
	}
	else
	{
		TutorialImage_->SetBrushFromTexture( nullptr );
	}
}

void UTutorialPageWidget::StartPlayback()
{
	if ( !GetWorld() || !TutorialImage_ || CurrentFrames_.Num() == 0 )
	{
		return;
	}

	const float delay = 1.0f / CurrentFPS_;

	CurrentFrameIndex_ = 0;

	if ( CurrentFrames_[0] )
	{
		TutorialImage_->SetBrushFromTexture( CurrentFrames_[0], true );
	}

	GetWorld()->GetTimerManager().SetTimer( FrameTimerHandle_, this, &UTutorialPageWidget::AdvanceFrame, delay, true );
}

void UTutorialPageWidget::StopPlayback()
{
	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( FrameTimerHandle_ );
	}
}

void UTutorialPageWidget::AdvanceFrame()
{
	if ( !bPlaying_ || !TutorialImage_ || CurrentFrames_.Num() == 0 )
	{
		return;
	}

	CurrentFrameIndex_ = ( CurrentFrameIndex_ + 1 ) % CurrentFrames_.Num();

	if ( CurrentFrames_[CurrentFrameIndex_] )
	{
		TutorialImage_->SetBrushFromTexture( CurrentFrames_[CurrentFrameIndex_], true );
	}
}