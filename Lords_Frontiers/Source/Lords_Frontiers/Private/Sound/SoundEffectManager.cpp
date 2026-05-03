#include "sound/SoundEffectManager.h"

#include "AudioDevice.h"
#include "Core/DefaultGameInstance.h"

#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "sound/AudioEventSource.h"

void USoundEffectManager::Initialize( FSubsystemCollectionBase& collection )
{
	if ( const auto* gi = Cast<UDefaultGameInstance>( UGameplayStatics::GetGameInstance( GetWorld() ) ) )
	{
		SoundData_ = gi->SoundData;
		DefaultAttenuation_ = gi->DefaultAttenuation;
	}
	Super::Initialize( collection );
}

void USoundEffectManager::Deinitialize()
{
	for ( TObjectPtr<UAudioComponent>& audio : ComponentsInUsePool_ )
	{
		ReleaseAudioComponent( audio );
	}
	ComponentsInUsePool_.Empty();

	for ( TObjectPtr<UAudioComponent>& audio : FreeComponentsPool_ )
	{
		if ( audio )
		{
			audio->Stop();
			audio->UnregisterComponent();
		}
	}
	FreeComponentsPool_.Empty();

	Super::Deinitialize();
}

void USoundEffectManager::RegisterActor( AActor* actor )
{
	if ( !IsValid( actor ) )
	{
		return;
	}

	IAudioEventSource* source = Cast<IAudioEventSource>( actor );
	if ( !source )
	{
		UE_LOG(
		    LogAudio, Warning, TEXT( "USoundEffectManager::RegisterActor — %s does not implement IAudioEventSource" ),
		    *actor->GetName()
		);
		return;
	}

	source->GetOnAudioEvent().AddDynamic( this, &USoundEffectManager::HandleAudioEvent );
	UE_LOG( LogAudio, Log, TEXT( "USoundEffectManager: actor %s registered" ), *actor->GetName() );
}

void USoundEffectManager::UnregisterActor( AActor* actor )
{
	if ( !IsValid( actor ) )
	{
		return;
	}

	IAudioEventSource* source = Cast<IAudioEventSource>( actor );
	if ( !source )
	{
		UE_LOG(
		    LogAudio, Warning, TEXT( "USoundEffectManager::UnregisterActor — %s does not implement IAudioEventSource" ),
		    *actor->GetName()
		);
		return;
	}

	source->GetOnAudioEvent().RemoveDynamic( this, &USoundEffectManager::HandleAudioEvent );
	UE_LOG( LogAudio, Log, TEXT( "USoundEffectManager: actor %s unregistered" ), *actor->GetName() );
}

void USoundEffectManager::HandleAudioEvent( FAudioEvent event )
{
	if ( !SoundData_ )
	{
		UE_LOG( LogAudio, Error, TEXT( "USoundEffectManager: SoundData asset is not set" ) );
		return;
	}

	const FSoundEntry* entry = SoundData_->FindByTag( event.SoundTag );
	if ( !entry )
	{
		UE_LOG(
		    LogAudio, Warning, TEXT( "USoundEffectManager: No entry found for tag '%s'" ), *event.SoundTag.ToString()
		);
		return;
	}

	if ( !entry->Sound )
	{
		return;
	}

	if ( entry->bIs3D )
	{
		Play3D( *entry, event.WorldLocation );
	}
	else
	{
		Play2D( *entry );
	}
}

TWeakObjectPtr<UAudioComponent> USoundEffectManager::AcquireAudioComponent()
{
	UAudioComponent* component;
	if ( FreeComponentsPool_.IsEmpty() )
	{
		component = NewObject<UAudioComponent>( GetWorld() );
		UE_LOG(
		    LogAudio, Log, TEXT( "USoundEffectManager: new UAudioComponent created. UAudioComponents in use: %d" ),
		    ComponentsInUsePool_.Num()
		);
	}
	else
	{
		auto it = FreeComponentsPool_.CreateIterator();
		auto value = *it;
		it.RemoveCurrent();
		component = value;
	}

	if ( component )
	{
		ComponentsInUsePool_.Add( component );
	}
	return component;
}

void USoundEffectManager::ReleaseAudioComponent( TWeakObjectPtr<UAudioComponent> component )
{
	if ( !component.IsValid() )
	{
		return;
	}

	component->Stop();

	component->OnAudioFinishedNative.Clear();
	component->OnAudioFinished.Clear();

	component->SetSound( nullptr );
	component->SetAttenuationSettings( nullptr );
	component->Deactivate();

	ComponentsInUsePool_.Remove( component.Get() );
	FreeComponentsPool_.Add( component.Get() );
}

void USoundEffectManager::OnSoundFinished( TWeakObjectPtr<UAudioComponent> component )
{
	ReleaseAudioComponent( component );
}

void USoundEffectManager::Play2D( const FSoundEntry& entry )
{
	UAudioComponent* audio = AcquireAudioComponent().Get();
	if ( !audio )
	{
		return;
	}

	audio->SetSound( entry.Sound );
	audio->SetVolumeMultiplier( FMath::RandRange( entry.VolumeRange.X, entry.VolumeRange.Y ) );
	audio->SetPitchMultiplier( FMath::RandRange( entry.PitchRange.X, entry.PitchRange.Y ) );
	audio->bAllowSpatialization = false;
	audio->AttenuationSettings = nullptr; // no attenuation for 2D

	TWeakObjectPtr<USoundEffectManager> weakThis = this;
	audio->OnAudioFinishedNative.AddLambda(
	    [weakThis]( UAudioComponent* component )
	    {
		    if ( weakThis.IsValid() && IsValid( component ) )
		    {
			    weakThis->OnSoundFinished( component );
		    }
	    }
	);

	audio->Play();
}

void USoundEffectManager::Play3D( const FSoundEntry& entry, const FVector& worldLocation )
{
	UAudioComponent* audio = AcquireAudioComponent().Get();
	if ( !audio )
	{
		return;
	}

	audio->SetWorldLocation( worldLocation );
	audio->SetSound( entry.Sound );
	audio->SetVolumeMultiplier( FMath::RandRange( entry.VolumeRange.X, entry.VolumeRange.Y ) );
	audio->SetPitchMultiplier( FMath::RandRange( entry.PitchRange.X, entry.PitchRange.Y ) );
	audio->bAllowSpatialization = true;
	if ( entry.Attenuation )
	{
		audio->AttenuationSettings = entry.Attenuation;
	}
	else
	{
		audio->AttenuationSettings = DefaultAttenuation_;
	}

	TWeakObjectPtr<USoundEffectManager> weakThis = this;
	audio->OnAudioFinishedNative.AddLambda(
	    [weakThis]( UAudioComponent* component )
	    {
		    if ( weakThis.IsValid() && IsValid( component ) )
		    {
			    weakThis->OnSoundFinished( component );
		    }
	    }
	);

	audio->Play();
}
