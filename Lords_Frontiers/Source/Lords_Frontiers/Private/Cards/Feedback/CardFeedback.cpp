#include "Cards/Feedback/CardFeedback.h"

#include "Cards/Feedback/CardFeedbackPopup.h"
#include "Core/DefaultGameInstance.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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

	UWorld* world = GEngine ? GEngine->GetWorldFromContextObject( worldContextObject, EGetWorldErrorMode::LogAndReturnNull ) : nullptr;
	if ( !world )
	{
		return;
	}

	TSubclassOf<ACardFeedbackPopup> popupClass = ResolvePopupClass( worldContextObject );
	if ( !popupClass )
	{
		UE_LOG(
		    LogCardFeedback, Warning,
		    TEXT( "CardFeedback: no popup class configured on GameInstance — skipping icon on %s" ),
		    *owner->GetName() );
		return;
	}

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = owner;

	const FVector spawnLoc = owner->GetActorLocation();
	ACardFeedbackPopup* popup = world->SpawnActor<ACardFeedbackPopup>( popupClass, spawnLoc, FRotator::ZeroRotator, params );
	if ( !popup )
	{
		return;
	}

	popup->Initialize( owner, icon, durationSeconds, floatHeight );
}

TSubclassOf<ACardFeedbackPopup> UCardFeedback::ResolvePopupClass( const UObject* worldContextObject )
{
	UGameInstance* gi = UGameplayStatics::GetGameInstance( worldContextObject );
	UDefaultGameInstance* defaultGI = Cast<UDefaultGameInstance>( gi );
	if ( !defaultGI )
	{
		return nullptr;
	}
	return defaultGI->GetCardFeedbackPopupClass();
}
