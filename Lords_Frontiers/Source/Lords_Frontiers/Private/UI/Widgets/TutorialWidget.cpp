#include "UI/Widgets/TutorialWidget.h"
#include "UI/Widgets/TutorialPageWidget.h"
#include "TimerManager.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Components/WidgetSwitcher.h"


void UTutorialWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BindUI();
}

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable( true );

	if ( !bPagesBuilt_ )
	{
		BuildPages();
	}

	if ( PagesSwitcher_ && PagesSwitcher_->GetChildrenCount() > 0 )
	{
		ActivatePage( 0 );
	}
	else
	{
		UpdateNavigationButtons();
	}

	SetKeyboardFocus();
}

void UTutorialWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UTutorialWidget::BindUI()
{
	if ( CloseButton_ )
	{
		CloseButton_->OnClicked.AddDynamic( this, &UTutorialWidget::HandleCloseClicked );
	}

	if ( BackdropButton_ )
	{
		BackdropButton_->OnClicked.AddDynamic( this, &UTutorialWidget::HandleBackdropClicked );
	}

	if ( NextButton_ )
	{
		NextButton_->OnClicked.AddDynamic( this, &UTutorialWidget::HandleNextClicked );
	}

	if ( PrevButton_ )
	{
		PrevButton_->OnClicked.AddDynamic( this, &UTutorialWidget::HandlePrevClicked );
	}
}

void UTutorialWidget::BuildPages()
{
	if ( !PagesSwitcher_ )
	{
		return;
	}

	PagesSwitcher_->ClearChildren();
	BuiltPages_.Reset();

	if ( !PageWidgetClass_ || !GetOwningPlayer() )
	{
		bPagesBuilt_ = true;
		return;
	}

	for ( const FTutorialPageData& pageData : Pages_ )
	{
		UTutorialPageWidget* pageWidget = CreateWidget<UTutorialPageWidget>( GetOwningPlayer(), PageWidgetClass_ );
		if ( !pageWidget )
		{
			continue;
		}

		pageWidget->SetData( pageData );
		pageWidget->OnPageDeactivated();

		PagesSwitcher_->AddChild( pageWidget );
		BuiltPages_.Add( pageWidget );
	}

	bPagesBuilt_ = true;
}

void UTutorialWidget::RebuildPages()
{
	bPagesBuilt_ = false;
	BuildPages();

	if ( PagesSwitcher_ && PagesSwitcher_->GetChildrenCount() > 0 )
	{
		ActivatePage( 0 );
	}
	else
	{
		UpdateNavigationButtons();
	}
}

void UTutorialWidget::ActivatePage( int32 newIndex )
{
	if ( !PagesSwitcher_ )
	{
		return;
	}

	const int32 count = PagesSwitcher_->GetChildrenCount();
	if ( count <= 0 )
	{
		CurrentPageIndex_ = INDEX_NONE;
		UpdateNavigationButtons();
		return;
	}

	const int32 clampedIndex = FMath::Clamp( newIndex, 0, count - 1 );

	if ( BuiltPages_.IsValidIndex( CurrentPageIndex_ ) && BuiltPages_[CurrentPageIndex_] )
	{
		BuiltPages_[CurrentPageIndex_]->OnPageDeactivated();
	}

	CurrentPageIndex_ = clampedIndex;
	PagesSwitcher_->SetActiveWidgetIndex( CurrentPageIndex_ );

	if ( BuiltPages_.IsValidIndex( CurrentPageIndex_ ) && BuiltPages_[CurrentPageIndex_] )
	{
		BuiltPages_[CurrentPageIndex_]->OnPageActivated();
	}

	UpdateNavigationButtons();
}

void UTutorialWidget::SetPage( int32 pageIndex )
{
	ActivatePage( pageIndex );
}

void UTutorialWidget::NextPage()
{
	SetPage( CurrentPageIndex_ + 1 );
}

void UTutorialWidget::PrevPage()
{
	SetPage( CurrentPageIndex_ - 1 );
}

void UTutorialWidget::UpdateNavigationButtons()
{
	const int32 count = PagesSwitcher_ ? PagesSwitcher_->GetChildrenCount() : 0;

	const bool bHasPages = count > 0;
	const bool bCanGoPrev = bHasPages && CurrentPageIndex_ > 0;
	const bool bCanGoNext = bHasPages && CurrentPageIndex_ < count - 1;

	if ( PrevButton_ )
	{
		PrevButton_->SetVisibility( bCanGoPrev ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}

	if ( NextButton_ )
	{
		NextButton_->SetVisibility( bCanGoNext ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}
}

FReply UTutorialWidget::NativeOnPreviewKeyDown( const FGeometry& inGeometry, const FKeyEvent& inKeyEvent )
{
	if ( inKeyEvent.GetKey() == EKeys::Escape )
	{
		CloseTutorial();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown( inGeometry, inKeyEvent );
}

void UTutorialWidget::HandleCloseClicked()
{
	CloseTutorial();
}

void UTutorialWidget::HandleBackdropClicked()
{
	CloseTutorial();
}

void UTutorialWidget::HandleNextClicked()
{
	NextPage();
}

void UTutorialWidget::HandlePrevClicked()
{
	PrevPage();
}

void UTutorialWidget::CloseTutorial()
{
	if ( BuiltPages_.IsValidIndex( CurrentPageIndex_ ) && BuiltPages_[CurrentPageIndex_] )
	{
		BuiltPages_[CurrentPageIndex_]->OnPageDeactivated();
	}

	OnTutorialClosed.Broadcast();
	RemoveFromParent();
}