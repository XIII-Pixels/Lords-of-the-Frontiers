#include "UI/Widgets/TextButtonWidget.h"

#include "Localization/GameLocalization.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RetainerBox.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void UTextButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLabel();
	ApplyGlow();
}

void UTextButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &UTextButtonWidget::HandleClicked );
		RootButton->OnHovered.AddDynamic( this, &UTextButtonWidget::HandleHovered );
		RootButton->OnUnhovered.AddDynamic( this, &UTextButtonWidget::HandleUnhovered );
	}

	bIsHovered_ = false;
	GlowProgress_ = 0.0f;
	ApplyLabel();
	ApplyGlow();
}

void UTextButtonWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &UTextButtonWidget::HandleClicked );
		RootButton->OnHovered.RemoveDynamic( this, &UTextButtonWidget::HandleHovered );
		RootButton->OnUnhovered.RemoveDynamic( this, &UTextButtonWidget::HandleUnhovered );
	}

	Super::NativeDestruct();
}

void UTextButtonWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	// No glow visuals bound — nothing to animate (keeps plain dialog buttons cost-free).
	if ( !GlowImage && !GlowRetainer )
	{
		return;
	}

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

void UTextButtonWidget::HandleHovered()
{
	bIsHovered_ = true;
	OnHovered.Broadcast();
}

void UTextButtonWidget::HandleUnhovered()
{
	bIsHovered_ = false;
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

void UTextButtonWidget::ApplyGlow()
{
	// Glow is opt-in: without a glow widget the button keeps its designer colours untouched.
	if ( !GlowImage && !GlowRetainer )
	{
		return;
	}

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
