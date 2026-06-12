#include "Lords_Frontiers/Public/UI/Widgets/ConstructionPanelWidget.h"

#include "Building/Construction/BuildManager.h"
#include "Core/CoreManager.h"
#include "Localization/GameLocalization.h"

#include "Animation/WidgetAnimation.h"

void UConstructionPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( ButtonCancel )
	{
		ButtonCancel->OnClicked.AddDynamic( this, &UConstructionPanelWidget::OnCancelClicked );
	}

	if ( HintText )
	{
		HintText->SetText( LF_LOC( "Construction.Hint" ) );
	}

	FWidgetAnimationDynamicEvent animFinished;
	animFinished.BindDynamic( this, &UConstructionPanelWidget::OnVisibilityAnimFinished );
	if ( ShowAnim )
	{
		BindToAnimationFinished( ShowAnim, animFinished );
	}
	if ( HideAnim )
	{
		BindToAnimationFinished( HideAnim, animFinished );
	}
}

void UConstructionPanelWidget::NativeDestruct()
{
	if ( ButtonCancel )
	{
		ButtonCancel->OnClicked.RemoveDynamic( this, &UConstructionPanelWidget::OnCancelClicked );
	}

	Super::NativeDestruct();
}

void UConstructionPanelWidget::OnCancelClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( ABuildManager* bM = core->GetBuildManager() )
		{
			if ( bM->IsPlacing() )
			{
				bM->CancelPlacing();
			}
		}
	}
}

void UConstructionPanelWidget::SetPanelVisible( bool bVisible )
{
	bWantsVisible_ = bVisible;

	if ( bVisible )
	{
		SetVisibility( ESlateVisibility::Visible );
	}

	PlayVisibilityAnim( bVisible );
}

void UConstructionPanelWidget::PlayVisibilityAnim( bool bVisible )
{
	if ( bVisible )
	{
		if ( ShowAnim )
		{
			PlayAnimationForward( ShowAnim );
		}
		else if ( HideAnim )
		{
			PlayAnimationReverse( HideAnim );
		}
	}
	else
	{
		if ( HideAnim )
		{
			PlayAnimationForward( HideAnim );
		}
		else if ( ShowAnim )
		{
			PlayAnimationReverse( ShowAnim );
		}
		else
		{
			SetVisibility( ESlateVisibility::Collapsed );
		}
	}
}

void UConstructionPanelWidget::OnVisibilityAnimFinished()
{
	if ( !bWantsVisible_ )
	{
		SetVisibility( ESlateVisibility::Collapsed );
	}
}
