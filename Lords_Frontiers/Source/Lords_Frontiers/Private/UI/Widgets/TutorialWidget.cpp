#include "UI/Widgets/TutorialWidget.h"

#include "TimerManager.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

void UTutorialWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BindUI();
}

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Нужно для ESC.
	SetIsFocusable( true );

	// На старте можно открыть первый шаг.
	if ( Steps.IsValidIndex( 0 ) )
	{
		ShowStep( 0 );
	}
}

void UTutorialWidget::NativeDestruct()
{
	StopPlayback();
	Super::NativeDestruct();
}

void UTutorialWidget::BindUI()
{
	if ( CloseButton )
	{
		CloseButton->OnClicked.RemoveAll( this );
		CloseButton->OnClicked.AddDynamic( this, &UTutorialWidget::HandleCloseClicked );
	}

	if ( BackdropButton )
	{
		BackdropButton->OnClicked.RemoveAll( this );
		BackdropButton->OnClicked.AddDynamic( this, &UTutorialWidget::HandleBackdropClicked );
	}
}

void UTutorialWidget::ShowStep( int32 StepIndex )
{
	if ( !Steps.IsValidIndex( StepIndex ) )
	{
		return;
	}

	StopPlayback();

	CurrentStepIndex = StepIndex;
	CurrentFrameIndex = 0;

	if ( TutorialText )
	{
		TutorialText->SetText( Steps[StepIndex].InstructionText );
	}

	StartPlayback();
}

void UTutorialWidget::StartPlayback()
{
	if ( !GetWorld() || !Steps.IsValidIndex( CurrentStepIndex ) )
	{
		return;
	}

	const TArray<TObjectPtr<UTexture2D>>& Frames = Steps[CurrentStepIndex].Frames;
	if ( Frames.IsEmpty() || !TutorialImage )
	{
		return;
	}

	const float SafeFPS = FMath::Max( FramesPerSecond, 1.0f );
	const float Delay = 1.0f / SafeFPS;

	// Сразу показать первый кадр.
	TutorialImage->SetBrushFromTexture( Frames[0], true );

	GetWorld()->GetTimerManager().SetTimer( FrameTimerHandle, this, &UTutorialWidget::AdvanceFrame, Delay, true );
}

void UTutorialWidget::StopPlayback()
{
	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( FrameTimerHandle );
	}
}

void UTutorialWidget::AdvanceFrame()
{
	if ( !Steps.IsValidIndex( CurrentStepIndex ) || !TutorialImage )
	{
		return;
	}

	const TArray<TObjectPtr<UTexture2D>>& Frames = Steps[CurrentStepIndex].Frames;
	if ( Frames.IsEmpty() )
	{
		return;
	}

	CurrentFrameIndex = ( CurrentFrameIndex + 1 ) % Frames.Num();
	TutorialImage->SetBrushFromTexture( Frames[CurrentFrameIndex], true );
}

FReply UTutorialWidget::NativeOnPreviewKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent )
{
	if ( InKeyEvent.GetKey() == EKeys::Escape )
	{
		CloseTutorial();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown( InGeometry, InKeyEvent );
}

void UTutorialWidget::HandleCloseClicked()
{
	CloseTutorial();
}

void UTutorialWidget::HandleBackdropClicked()
{
	CloseTutorial();
}

void UTutorialWidget::CloseTutorial()
{
	StopPlayback();
	OnTutorialClosed.Broadcast();
	RemoveFromParent();
}