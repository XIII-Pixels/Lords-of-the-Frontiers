#include "UI/CursorAnim/CursorAnimFrameImage.h"

#include "UI/CursorAnim/CursorAnimationSubsystem.h"

#include "InputCoreTypes.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Widgets/Images/SImage.h"

void UCursorAnimFrameImage::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if ( MyImage.IsValid() )
	{
		const FPointerEventHandler handler =
		    FPointerEventHandler::CreateUObject( this, &UCursorAnimFrameImage::HandleMouseButtonDown );
		MyImage->SetOnMouseButtonDown( handler );

		MyImage->SetOnMouseDoubleClick( handler );
	}
}

FReply UCursorAnimFrameImage::HandleMouseButtonDown( const FGeometry& , const FPointerEvent& mouseEvent )
{
	if ( mouseEvent.GetEffectingButton() != EKeys::LeftMouseButton )
	{
		return FReply::Unhandled();
	}

	if ( UCursorAnimationSubsystem* subsystem = UCursorAnimationSubsystem::Get( this ) )
	{
		subsystem->PlayAtScreenPosition( AnimationTag, mouseEvent.GetScreenSpacePosition() );
	}

	return FReply::Handled();
}
