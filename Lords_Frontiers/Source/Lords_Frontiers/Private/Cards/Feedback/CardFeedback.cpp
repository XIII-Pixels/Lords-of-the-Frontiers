#include "Cards/Feedback/CardFeedback.h"

#include "Cards/Visuals/CardVisualSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardFeedback, Log, All );

void UCardFeedback::ShowIconOnActor(
	const UObject* worldContextObject,
	AActor* owner,
	UTexture2D* icon,
	float durationSeconds,
	float floatHeight )
{
	if ( !owner || !icon )
	{
		return;
	}

	UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( worldContextObject );
	if ( !visuals )
	{
		UE_LOG( LogCardFeedback, Warning, TEXT( "ShowIconOnActor: CardVisualSubsystem unavailable" ) );
		return;
	}

	FCardVisualConfig config;
	config.Icon.Icon = icon;
	config.Icon.ShowOn = ECardVisualTarget::Owner;
	config.Icon.Mode = ECardVisualIconMode::Popup;
	config.Icon.PopupDurationSeconds = durationSeconds;
	config.Icon.PopupFloatHeight = floatHeight;

	visuals->PlayOneShot( config, owner, nullptr );
}

void UCardFeedback::PlayVisuals(
	const UObject* worldContextObject,
	const FCardVisualConfig& config,
	AActor* owner,
	AActor* target )
{
	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( worldContextObject ) )
	{
		visuals->PlayOneShot( config, owner, target );
	}
}

FCardVisualHandle UCardFeedback::BeginStickyVisual(
	const UObject* worldContextObject,
	const FCardVisualConfig& config,
	AActor* owner,
	AActor* target )
{
	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( worldContextObject ) )
	{
		return visuals->BeginSticky( config, owner, target );
	}
	return FCardVisualHandle();
}

void UCardFeedback::StopStickyVisual( const UObject* worldContextObject, FCardVisualHandle handle )
{
	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( worldContextObject ) )
	{
		visuals->EndSticky( handle );
	}
}
