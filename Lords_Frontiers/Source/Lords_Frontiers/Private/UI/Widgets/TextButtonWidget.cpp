#include "UI/Widgets/TextButtonWidget.h"

#include "Localization/GameLocalization.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTextButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLabel();
}

void UTextButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &UTextButtonWidget::HandleClicked );
	}

	ApplyLabel();
}

void UTextButtonWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &UTextButtonWidget::HandleClicked );
	}

	Super::NativeDestruct();
}

void UTextButtonWidget::SetLabelKey( FName key )
{
	LabelKey = key;
	ApplyLabel();
}

void UTextButtonWidget::SetLabelText( FText text )
{
	LabelText = text;
	ApplyLabel();
}

void UTextButtonWidget::HandleClicked()
{
	OnClicked.Broadcast();
}

void UTextButtonWidget::ApplyLabel()
{
	if ( !ButtonText )
	{
		return;
	}

	if ( !LabelKey.IsNone() )
	{
		ButtonText->SetText( FText::FromStringTable( LordsFrontiersLoc::GetTableId(), LabelKey.ToString() ) );
	}
	else if ( !LabelText.IsEmpty() )
	{
		ButtonText->SetText( LabelText );
	}
}
