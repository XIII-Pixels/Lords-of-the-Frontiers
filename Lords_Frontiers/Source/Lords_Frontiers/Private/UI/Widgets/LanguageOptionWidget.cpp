#include "UI/Widgets/LanguageOptionWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void ULanguageOptionWidget::Setup( const FLanguageOption& option, const FText& resolvedName, int32 optionIndex )
{
	OptionIndex_ = optionIndex;

	if ( NameText )
	{
		NameText->SetText( resolvedName );
	}

	ULanguageSwitchWidget::ApplyFlagBrush( FlagImage, option );
}

void ULanguageOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &ULanguageOptionWidget::HandleClicked );
	}
}

void ULanguageOptionWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &ULanguageOptionWidget::HandleClicked );
	}

	Super::NativeDestruct();
}

void ULanguageOptionWidget::HandleClicked()
{
	OnPicked.Broadcast( OptionIndex_ );
}
