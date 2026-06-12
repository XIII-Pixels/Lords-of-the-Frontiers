#include "Lords_Frontiers/Public/UI/Widgets/EndTurnButtonWidget.h"

#include "Core/CoreManager.h"
#include "Localization/GameLocalization.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void UEndTurnButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLabel();
	ApplyGlow();
}

void UEndTurnButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &UEndTurnButtonWidget::HandleClicked );
		RootButton->OnHovered.AddDynamic( this, &UEndTurnButtonWidget::HandleHovered );
		RootButton->OnUnhovered.AddDynamic( this, &UEndTurnButtonWidget::HandleUnhovered );
	}

	if ( UGameLoopManager* gameLoop = GetGameLoop() )
	{
		gameLoop->OnPhaseChanged.AddUniqueDynamic( this, &UEndTurnButtonWidget::HandlePhaseChanged );
		UpdatePhaseVisibility( gameLoop->GetCurrentPhase() );
	}

	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->RegisterObject( this );
			}
		}
	}

	bIsHovered_ = false;
	GlowProgress_ = 0.0f;
	ApplyLabel();
	ApplyGlow();
}

void UEndTurnButtonWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &UEndTurnButtonWidget::HandleClicked );
		RootButton->OnHovered.RemoveDynamic( this, &UEndTurnButtonWidget::HandleHovered );
		RootButton->OnUnhovered.RemoveDynamic( this, &UEndTurnButtonWidget::HandleUnhovered );
	}

	if ( UGameLoopManager* gameLoop = GetGameLoop() )
	{
		gameLoop->OnPhaseChanged.RemoveDynamic( this, &UEndTurnButtonWidget::HandlePhaseChanged );
	}

	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->UnregisterObject( this );
			}
		}
	}

	Super::NativeDestruct();
}

void UEndTurnButtonWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
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

void UEndTurnButtonWidget::SetLabelKey( FName key )
{
	LabelKey = key;
	ApplyLabel();
}

void UEndTurnButtonWidget::SetLabelText( FText text )
{
	LabelText = text;
	ApplyLabel();
}

void UEndTurnButtonWidget::HandleClicked()
{
	if ( UGameLoopManager* gameLoop = GetGameLoop() )
	{
		gameLoop->EndBuildTurn();
	}

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_ENDTURN_CLICKED } );
	OnEndTurnRequested.Broadcast();
}

void UEndTurnButtonWidget::HandleHovered()
{
	bIsHovered_ = true;
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_ENDTURN_HOVERED } );
}

void UEndTurnButtonWidget::HandleUnhovered()
{
	bIsHovered_ = false;
}

void UEndTurnButtonWidget::HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	UpdatePhaseVisibility( newPhase );
}

void UEndTurnButtonWidget::ApplyLabel()
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

void UEndTurnButtonWidget::ApplyGlow()
{
	if ( GlowImage )
	{
		GlowImage->SetColorAndOpacity( FMath::Lerp( GlowColorIdle, GlowColorHovered, GlowProgress_ ) );
	}

	OnGlowProgressChanged( GlowProgress_ );
}

void UEndTurnButtonWidget::UpdatePhaseVisibility( EGameLoopPhase phase )
{
	if ( !bAutoHideOutsideBuildPhase )
	{
		return;
	}

	const bool bShow = ( phase == EGameLoopPhase::Building );
	SetVisibility( bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
}

UGameLoopManager* UEndTurnButtonWidget::GetGameLoop() const
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		return core->GetGameLoop();
	}
	return nullptr;
}
