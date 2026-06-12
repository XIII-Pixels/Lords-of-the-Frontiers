#include "UI/Widgets/MainMenuButtonWidget.h"

#include "Localization/GameLocalization.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RetainerBox.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMainMenuButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLabel();
	ApplyGlow();
}

void UMainMenuButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &UMainMenuButtonWidget::HandleClicked );
		RootButton->OnHovered.AddDynamic( this, &UMainMenuButtonWidget::HandleHovered );
		RootButton->OnUnhovered.AddDynamic( this, &UMainMenuButtonWidget::HandleUnhovered );
	}

	bIsHovered_ = false;
	GlowProgress_ = 0.0f;
	ApplyLabel();
	ApplyGlow();
}

void UMainMenuButtonWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &UMainMenuButtonWidget::HandleClicked );
		RootButton->OnHovered.RemoveDynamic( this, &UMainMenuButtonWidget::HandleHovered );
		RootButton->OnUnhovered.RemoveDynamic( this, &UMainMenuButtonWidget::HandleUnhovered );
	}

	Super::NativeDestruct();
}

void UMainMenuButtonWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	const float target = bIsHovered_ ? 1.0f : 0.0f;
	if ( FMath::IsNearlyEqual( GlowProgress_, target ) )
	{
		return;
	}

	if ( GlowTransitionTime <= KINDA_SMALL_NUMBER )
	{
		GlowProgress_ = target;
	}
	else
	{
		GlowProgress_ = FMath::FInterpConstantTo( GlowProgress_, target, inDeltaTime, 1.0f / GlowTransitionTime );
	}

	ApplyGlow();
}

void UMainMenuButtonWidget::SetLabelKey( FName key )
{
	LabelKey = key;
	ApplyLabel();
}

void UMainMenuButtonWidget::SetLabelText( FText text )
{
	LabelText = text;
	ApplyLabel();
}

void UMainMenuButtonWidget::SetAction( EMainMenuButtonAction newAction )
{
	Action = newAction;
}

void UMainMenuButtonWidget::HandleClicked()
{
	OnClicked.Broadcast( Action );
}

void UMainMenuButtonWidget::HandleHovered()
{
	bIsHovered_ = true;
	OnHovered.Broadcast( Action );
}

void UMainMenuButtonWidget::HandleUnhovered()
{
	bIsHovered_ = false;
	OnUnhovered.Broadcast( Action );
}

void UMainMenuButtonWidget::ApplyLabel()
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

void UMainMenuButtonWidget::ApplyGlow()
{
	const FLinearColor glowColor = FMath::Lerp( GlowColorIdle, GlowColorHovered, GlowProgress_ );

	if ( GlowImage )
	{
		GlowImage->SetColorAndOpacity( glowColor );
	}

	if ( GlowRetainer )
	{
		if ( UMaterialInstanceDynamic* material = GlowRetainer->GetEffectMaterial() )
		{
			material->SetVectorParameterValue( GlowColorParameter, glowColor );
			material->SetScalarParameterValue( GlowProgressParameter, GlowProgress_ );
		}
	}

	if ( ButtonText )
	{
		ButtonText->SetColorAndOpacity( FMath::Lerp( TextColorIdle, TextColorHovered, GlowProgress_ ) );

		if ( bGlowTextOutline )
		{
			FSlateFontInfo font = ButtonText->GetFont();
			font.OutlineSettings.OutlineColor = glowColor;
			ButtonText->SetFont( font );
		}
	}

	OnGlowProgressChanged( GlowProgress_ );
}
