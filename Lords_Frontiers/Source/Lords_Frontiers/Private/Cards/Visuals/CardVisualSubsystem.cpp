#include "Cards/Visuals/CardVisualSubsystem.h"

#include "Cards/Feedback/CardFeedbackPopup.h"
#include "Cards/Feedback/CardIconStrip.h"
#include "Core/DefaultGameInstance.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardVisuals, Log, All );

UCardVisualSubsystem* UCardVisualSubsystem::Get( const UObject* worldContextObject )
{
	if ( !worldContextObject || !GEngine )
	{
		return nullptr;
	}

	UWorld* world = GEngine->GetWorldFromContextObject( worldContextObject, EGetWorldErrorMode::LogAndReturnNull );
	if ( !world )
	{
		return nullptr;
	}

	return world->GetSubsystem<UCardVisualSubsystem>();
}

void UCardVisualSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );
	UE_LOG( LogCardVisuals, Log, TEXT( "CardVisualSubsystem initialized" ) );
}

void UCardVisualSubsystem::Deinitialize()
{
	if ( UWorld* world = GetWorld() )
	{
		FTimerManager& timers = world->GetTimerManager();
		for ( FTimerHandle& handle : PendingTimers_ )
		{
			timers.ClearTimer( handle );
		}
	}
	PendingTimers_.Empty();

	for ( FCardStickyRecord& record : Stickies_ )
	{
		if ( UNiagaraComponent* niagara = record.NiagaraComponent.Get() )
		{
			niagara->ReleaseToPool();
		}
	}
	Stickies_.Empty();

	for ( const TObjectPtr<ACardIconStrip>& strip : ActiveStripsList_ )
	{
		if ( strip )
		{
			strip->DeactivateToPool();
		}
	}
	ActiveStripsList_.Empty();
	ActiveStripsByHost_.Empty();

	for ( const TObjectPtr<ACardIconStrip>& strip : FreeStripPool_ )
	{
		if ( strip )
		{
			strip->Destroy();
		}
	}
	FreeStripPool_.Empty();

	for ( const TObjectPtr<ACardFeedbackPopup>& icon : InUseIcons_ )
	{
		if ( icon )
		{
			icon->DeactivateToPool();
		}
	}
	InUseIcons_.Empty();

	for ( const TObjectPtr<ACardFeedbackPopup>& icon : FreeIconPool_ )
	{
		if ( icon )
		{
			icon->Destroy();
		}
	}
	FreeIconPool_.Empty();

	Super::Deinitialize();
}

void UCardVisualSubsystem::SetPopupClassOverride( TSubclassOf<ACardFeedbackPopup> popupClass )
{
	PopupClassOverride_ = popupClass;
}

void UCardVisualSubsystem::PrewarmIconPool( int32 count )
{
	UWorld* world = GetWorld();
	if ( !world || count <= 0 )
	{
		return;
	}

	TSubclassOf<ACardFeedbackPopup> popupClass = ResolvePopupClass();
	if ( !popupClass )
	{
		UE_LOG( LogCardVisuals, Warning, TEXT( "PrewarmIconPool: popup class not configured" ) );
		return;
	}

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for ( int32 i = 0; i < count; ++i )
	{
		ACardFeedbackPopup* popup = world->SpawnActor<ACardFeedbackPopup>(
			popupClass, FVector::ZeroVector, FRotator::ZeroRotator, params );
		if ( !popup )
		{
			continue;
		}
		popup->DeactivateToPool();
		popup->OnPopupFinished.AddUObject( this, &UCardVisualSubsystem::ReleaseIconActor );
		FreeIconPool_.Add( popup );
	}
}

void UCardVisualSubsystem::PlayOneShot( const FCardVisualConfig& config, AActor* owner, AActor* target )
{
	if ( config.Icon.Mode != ECardVisualIconMode::Sticky )
	{
		PlayIcon( config.Icon, owner, target, false, INDEX_NONE );
	}
	if ( !config.Niagara.bLoop )
	{
		PlayNiagara( config.Niagara, owner, target, false, INDEX_NONE );
	}
}

FCardVisualHandle UCardVisualSubsystem::BeginSticky(
	const FCardVisualConfig& config, AActor* owner, AActor* target )
{
	FCardVisualHandle handle;
	const int32 id = AllocateStickyId();

	FCardStickyRecord record;
	record.Id = id;
	record.Host = owner;
	Stickies_.Add( record );

	if ( config.Icon.Mode == ECardVisualIconMode::Sticky )
	{
		PlayIcon( config.Icon, owner, target, true, id );
	}
	else
	{
		PlayIcon( config.Icon, owner, target, false, INDEX_NONE );
	}

	if ( config.Niagara.bLoop )
	{
		PlayNiagara( config.Niagara, owner, target, true, id );
	}
	else
	{
		PlayNiagara( config.Niagara, owner, target, false, INDEX_NONE );
	}

	handle.Id = id;
	return handle;
}

void UCardVisualSubsystem::EndSticky( FCardVisualHandle handle )
{
	if ( !handle.IsValid() )
	{
		return;
	}

	int32 foundIndex = INDEX_NONE;
	for ( int32 i = 0; i < Stickies_.Num(); ++i )
	{
		if ( Stickies_[i].Id == handle.Id )
		{
			foundIndex = i;
			break;
		}
	}

	if ( foundIndex == INDEX_NONE )
	{
		return;
	}

	FCardStickyRecord& record = Stickies_[foundIndex];

	if ( ACardIconStrip* strip = record.IconStrip.Get() )
	{
		const bool bEmpty = strip->RemoveIconSlot( record.IconSlotId, 0.f );
		if ( bEmpty )
		{
			ReleaseStrip( strip );
		}
	}
	if ( UNiagaraComponent* niagara = record.NiagaraComponent.Get() )
	{
		niagara->Deactivate();
		niagara->ReleaseToPool();
	}

	Stickies_.RemoveAt( foundIndex );
}

ACardFeedbackPopup* UCardVisualSubsystem::AcquireIconActor()
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		return nullptr;
	}

	while ( FreeIconPool_.Num() > 0 )
	{
		TObjectPtr<ACardFeedbackPopup> pooled = FreeIconPool_.Pop( EAllowShrinking::No );
		if ( IsValid( pooled ) )
		{
			InUseIcons_.Add( pooled );
			return pooled;
		}
	}

	TSubclassOf<ACardFeedbackPopup> popupClass = ResolvePopupClass();
	if ( !popupClass )
	{
		UE_LOG( LogCardVisuals, Warning, TEXT( "AcquireIconActor: popup class not configured" ) );
		return nullptr;
	}

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACardFeedbackPopup* fresh = world->SpawnActor<ACardFeedbackPopup>(
		popupClass, FVector::ZeroVector, FRotator::ZeroRotator, params );
	if ( !fresh )
	{
		return nullptr;
	}

	fresh->OnPopupFinished.AddUObject( this, &UCardVisualSubsystem::ReleaseIconActor );
	InUseIcons_.Add( fresh );
	return fresh;
}

void UCardVisualSubsystem::ReleaseIconActor( ACardFeedbackPopup* actor )
{
	if ( !actor )
	{
		return;
	}

	InUseIcons_.Remove( actor );
	actor->DeactivateToPool();

	FreeIconPool_.Add( actor );
}

TSubclassOf<ACardFeedbackPopup> UCardVisualSubsystem::ResolvePopupClass() const
{
	if ( PopupClassOverride_ )
	{
		return PopupClassOverride_;
	}

	UGameInstance* gi = UGameplayStatics::GetGameInstance( GetWorld() );
	UDefaultGameInstance* defaultGI = Cast<UDefaultGameInstance>( gi );
	return defaultGI ? defaultGI->GetCardFeedbackPopupClass() : nullptr;
}

TSubclassOf<ACardIconStrip> UCardVisualSubsystem::ResolveStripClass() const
{
	if ( StripClassOverride_ )
	{
		return StripClassOverride_;
	}

	UGameInstance* gi = UGameplayStatics::GetGameInstance( GetWorld() );
	UDefaultGameInstance* defaultGI = Cast<UDefaultGameInstance>( gi );
	return defaultGI ? defaultGI->GetCardIconStripClass() : nullptr;
}

ACardIconStrip* UCardVisualSubsystem::GetOrCreateStrip( AActor* host )
{
	if ( !host )
	{
		return nullptr;
	}

	if ( ACardIconStrip** existing = ActiveStripsByHost_.Find( host ) )
	{
		if ( IsValid( *existing ) )
		{
			return *existing;
		}
		ActiveStripsByHost_.Remove( host );
	}

	ACardIconStrip* strip = nullptr;
	while ( FreeStripPool_.Num() > 0 )
	{
		TObjectPtr<ACardIconStrip> pooled = FreeStripPool_.Pop( EAllowShrinking::No );
		if ( IsValid( pooled ) )
		{
			strip = pooled;
			break;
		}
	}

	if ( !strip )
	{
		UWorld* world = GetWorld();
		TSubclassOf<ACardIconStrip> stripClass = ResolveStripClass();
		if ( !world || !stripClass )
		{
			UE_LOG( LogCardVisuals, Warning,
				TEXT( "GetOrCreateStrip: strip class not configured on UDefaultGameInstance" ) );
			return nullptr;
		}

		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		strip = world->SpawnActor<ACardIconStrip>(
			stripClass, FVector::ZeroVector, FRotator::ZeroRotator, params );
		if ( !strip )
		{
			return nullptr;
		}
	}

	strip->ActivateOn( host );
	ActiveStripsList_.Add( strip );
	ActiveStripsByHost_.Add( host, strip );
	return strip;
}

void UCardVisualSubsystem::ReleaseStrip( ACardIconStrip* strip )
{
	if ( !strip )
	{
		return;
	}

	if ( AActor* host = strip->GetHost() )
	{
		ActiveStripsByHost_.Remove( host );
	}
	ActiveStripsList_.Remove( strip );

	strip->DeactivateToPool();
	FreeStripPool_.Add( strip );
}

void UCardVisualSubsystem::ResolveTargetHosts(
	ECardVisualTarget target, AActor* owner, AActor* hitTarget, TArray<AActor*>& outHosts ) const
{
	switch ( target )
	{
		case ECardVisualTarget::Owner:
			if ( owner )
			{
				outHosts.Add( owner );
			}
			break;
		case ECardVisualTarget::Target:
			if ( hitTarget )
			{
				outHosts.Add( hitTarget );
			}
			break;
		case ECardVisualTarget::Both:
			if ( owner )
			{
				outHosts.Add( owner );
			}
			if ( hitTarget && hitTarget != owner )
			{
				outHosts.Add( hitTarget );
			}
			break;
		case ECardVisualTarget::None:
		default:
			break;
	}
}

void UCardVisualSubsystem::PlayIcon(
	const FCardIconSpec& spec, AActor* owner, AActor* target, bool bSticky, int32 stickyId )
{
	if ( spec.Icon.IsNull() || spec.ShowOn == ECardVisualTarget::None )
	{
		return;
	}

	TArray<AActor*> hosts;
	ResolveTargetHosts( spec.ShowOn, owner, target, hosts );
	if ( hosts.Num() == 0 )
	{
		return;
	}

	for ( AActor* host : hosts )
	{
		if ( spec.DelaySeconds <= 0.f )
		{
			SpawnIconNow( spec, host, bSticky, stickyId );
			continue;
		}

		UWorld* world = GetWorld();
		if ( !world )
		{
			continue;
		}

		TWeakObjectPtr<UCardVisualSubsystem> weakSelf( this );
		TWeakObjectPtr<AActor> weakHost( host );
		FCardIconSpec specCopy = spec;
		const bool bStickyCopy = bSticky;
		const int32 stickyIdCopy = stickyId;

		FTimerHandle handle;
		world->GetTimerManager().SetTimer( handle, FTimerDelegate::CreateLambda(
			[weakSelf, weakHost, specCopy, bStickyCopy, stickyIdCopy]()
			{
				UCardVisualSubsystem* self = weakSelf.Get();
				AActor* liveHost = weakHost.Get();
				if ( !self || !liveHost )
				{
					return;
				}
				self->SpawnIconNow( specCopy, liveHost, bStickyCopy, stickyIdCopy );
			} ), spec.DelaySeconds, false );
		PendingTimers_.Add( handle );
	}
}

void UCardVisualSubsystem::SpawnIconNow(
	const FCardIconSpec& spec, AActor* host, bool bSticky, int32 stickyId )
{
	if ( !host )
	{
		return;
	}

	UTexture2D* iconTexture = spec.Icon.LoadSynchronous();
	if ( !iconTexture )
	{
		return;
	}

	const bool bActuallySticky = bSticky && spec.Mode == ECardVisualIconMode::Sticky;

	if ( bActuallySticky )
	{
		ACardIconStrip* strip = GetOrCreateStrip( host );
		if ( !strip )
		{
			return;
		}

		const int32 slotId = strip->AddIconSlot( iconTexture, spec.BaseHeightOffset );
		if ( slotId == INDEX_NONE )
		{
			return;
		}

		FCardStickyRecord* record = FindSticky( stickyId );
		if ( record )
		{
			record->IconStrip = strip;
			record->IconSlotId = slotId;
			record->Host = host;
		}
		return;
	}

	ACardFeedbackPopup* popup = AcquireIconActor();
	if ( !popup )
	{
		return;
	}

	popup->ActivatePopup( host, iconTexture, spec.PopupDurationSeconds, spec.PopupFloatHeight, spec.BaseHeightOffset );
}

void UCardVisualSubsystem::PlayNiagara(
	const FCardNiagaraSpec& spec, AActor* owner, AActor* target, bool bSticky, int32 stickyId )
{
	if ( spec.System.IsNull() || spec.SpawnOn == ECardVisualTarget::None )
	{
		return;
	}

	TArray<AActor*> hosts;
	ResolveTargetHosts( spec.SpawnOn, owner, target, hosts );
	if ( hosts.Num() == 0 )
	{
		return;
	}

	for ( AActor* host : hosts )
	{
		if ( spec.DelaySeconds <= 0.f )
		{
			SpawnNiagaraNow( spec, host, bSticky, stickyId );
			continue;
		}

		UWorld* world = GetWorld();
		if ( !world )
		{
			continue;
		}

		TWeakObjectPtr<UCardVisualSubsystem> weakSelf( this );
		TWeakObjectPtr<AActor> weakHost( host );
		FCardNiagaraSpec specCopy = spec;
		const bool bStickyCopy = bSticky;
		const int32 stickyIdCopy = stickyId;

		FTimerHandle handle;
		world->GetTimerManager().SetTimer( handle, FTimerDelegate::CreateLambda(
			[weakSelf, weakHost, specCopy, bStickyCopy, stickyIdCopy]()
			{
				UCardVisualSubsystem* self = weakSelf.Get();
				AActor* liveHost = weakHost.Get();
				if ( !self || !liveHost )
				{
					return;
				}
				self->SpawnNiagaraNow( specCopy, liveHost, bStickyCopy, stickyIdCopy );
			} ), spec.DelaySeconds, false );
		PendingTimers_.Add( handle );
	}
}

void UCardVisualSubsystem::SpawnNiagaraNow(
	const FCardNiagaraSpec& spec, AActor* host, bool bSticky, int32 stickyId )
{
	if ( !host )
	{
		return;
	}

	UNiagaraSystem* system = spec.System.LoadSynchronous();
	if ( !system )
	{
		return;
	}

	const bool bRetainNiagara = bSticky && spec.bLoop;
	const ENCPoolMethod pooling = bRetainNiagara
		? ENCPoolMethod::ManualRelease
		: ENCPoolMethod::AutoRelease;

	UNiagaraComponent* component = nullptr;
	if ( spec.bAttachToHost && host->GetRootComponent() )
	{
		component = UNiagaraFunctionLibrary::SpawnSystemAttached(
			system,
			host->GetRootComponent(),
			NAME_None,
			spec.LocationOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			!bRetainNiagara,
			true,
			pooling );
	}
	else
	{
		const FVector worldLoc = host->GetActorLocation() + spec.LocationOffset;
		component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			system,
			worldLoc,
			FRotator::ZeroRotator,
			FVector( 1.f ),
			!bRetainNiagara,
			true,
			pooling );
	}

	if ( !component )
	{
		return;
	}

	if ( bRetainNiagara )
	{
		FCardStickyRecord* record = FindSticky( stickyId );
		if ( record )
		{
			record->NiagaraComponent = component;
		}
	}
}

int32 UCardVisualSubsystem::AllocateStickyId()
{
	return NextStickyId_++;
}

FCardStickyRecord* UCardVisualSubsystem::FindSticky( int32 id )
{
	if ( id == INDEX_NONE )
	{
		return nullptr;
	}
	for ( FCardStickyRecord& record : Stickies_ )
	{
		if ( record.Id == id )
		{
			return &record;
		}
	}
	return nullptr;
}
