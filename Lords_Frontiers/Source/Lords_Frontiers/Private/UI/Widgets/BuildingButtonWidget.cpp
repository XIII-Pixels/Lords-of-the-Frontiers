#include "UI/Widgets/BuildingButtonWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UBuildingButtonWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ApplyIcon();
}

void UBuildingButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &UBuildingButtonWidget::HandleClicked );
		RootButton->OnHovered.AddDynamic( this, &UBuildingButtonWidget::HandleHovered );
		RootButton->OnUnhovered.AddDynamic( this, &UBuildingButtonWidget::HandleUnhovered );
	}

	ApplyIcon();
}

void UBuildingButtonWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &UBuildingButtonWidget::HandleClicked );
		RootButton->OnHovered.RemoveDynamic( this, &UBuildingButtonWidget::HandleHovered );
		RootButton->OnUnhovered.RemoveDynamic( this, &UBuildingButtonWidget::HandleUnhovered );
	}

	Super::NativeDestruct();
}

void UBuildingButtonWidget::HandleClicked()
{
	OnClicked.Broadcast();
}

void UBuildingButtonWidget::HandleHovered()
{
	OnHovered.Broadcast();
}

void UBuildingButtonWidget::HandleUnhovered()
{
	OnUnhovered.Broadcast();
}

void UBuildingButtonWidget::SetBackgroundColor( FLinearColor color )
{
	if ( RootButton )
	{
		RootButton->SetBackgroundColor( color );
	}
}

FLinearColor UBuildingButtonWidget::GetBackgroundColor() const
{
	return RootButton ? RootButton->GetBackgroundColor() : FLinearColor::White;
}

void UBuildingButtonWidget::SetIconTexture( UTexture2D* texture )
{
	IconTexture = texture;
	ApplyIcon();
}

void UBuildingButtonWidget::ApplyIcon()
{
	if ( BuildingIcon && IconTexture )
	{
		BuildingIcon->SetBrushFromTexture( IconTexture );
	}
}
