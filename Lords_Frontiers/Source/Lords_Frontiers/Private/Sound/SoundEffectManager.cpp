#include "sound/SoundEffectManager.h"

#include "AudioDevice.h"
#include "Core/DefaultGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/Data/SoundDataAsset.h"
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
	for ( TObjectPtr<UAudioComponent> audio : ComponentsInUsePool_ )
	{
		if ( IsValid( audio ) )
		{
			audio->OnAudioFinishedNative.Clear();
			audio->OnAudioFinished.Clear();
		}
	}
	ComponentsInUsePool_.Empty();

	for ( TObjectPtr<UAudioComponent>& audio : FreeComponentsPool_ )
	{
		if ( IsValid( audio ) )
		{
			audio->Stop();
			audio->UnregisterComponent();
		}
	}
	FreeComponentsPool_.Empty();

	Super::Deinitialize();
}

void USoundEffectManager::RegisterObject( UObject* object )
{
	if ( !IsValid( object ) )
	{
		return;
	}

	IAudioEventSource* source = Cast<IAudioEventSource>( object );
	if ( !source )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "USoundEffectManager: %s does not implement IAudioEventSource" ), *object->GetName()
		);
		return;
	}

	source->GetOnAudioEvent().AddUniqueDynamic( this, &USoundEffectManager::HandleAudioEvent );
	UE_LOG( LogTemp, Log, TEXT( "USoundEffectManager: actor %s registered" ), *object->GetName() );
}

void USoundEffectManager::UnregisterObject( UObject* object )
{
	if ( !IsValid( object ) )
	{
		return;
	}

	IAudioEventSource* source = Cast<IAudioEventSource>( object );
	if ( !source )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "USoundEffectManager: %s does not implement IAudioEventSource" ), *object->GetName()
		);
		return;
	}

	source->GetOnAudioEvent().RemoveDynamic( this, &USoundEffectManager::HandleAudioEvent );
	UE_LOG( LogTemp, Log, TEXT( "USoundEffectManager: actor %s unregistered" ), *object->GetName() );
}

void USoundEffectManager::HandleAudioEvent( FAudioEvent event )
{
	if ( !SoundData_ )
	{
		UE_LOG( LogTemp, Error, TEXT( "USoundEffectManager: SoundData asset is not set" ) );
		return;
	}

	const FSoundEntry* entry = SoundData_->FindByTag( event.SoundTag );
	if ( !entry )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "USoundEffectManager: No entry found for tag '%s'" ), *event.SoundTag.ToString()
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
		    LogTemp, Log, TEXT( "USoundEffectManager: new UAudioComponent created. UAudioComponents in use: %d" ),
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
	if ( !entry.Sound )
	{
		return;
	}

	const float volume = FMath::RandRange( entry.VolumeRange.X, entry.VolumeRange.Y );
	const float pitch  = FMath::RandRange( entry.PitchRange.X, entry.PitchRange.Y );

	UAudioComponent* audio = UGameplayStatics::SpawnSound2D(
		this,
		entry.Sound,
		volume,
		pitch,
		0.0f,
		nullptr,
		true,
		true
	);

	if ( !audio )
	{
		UE_LOG( LogTemp, Error, TEXT( "USoundEffectManager: failed to play 2D sound: %s" ), *entry.Sound->GetName() );
		return;
	}

	UE_LOG( LogTemp, Log, TEXT( "USoundEffectManager: playing 2D sound: %s" ), *entry.Sound->GetName() );
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
	else if ( DefaultAttenuation_ )
	{
		audio->AttenuationSettings = DefaultAttenuation_;
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "USoundEffectManager: playing 3D sound without attenuation" ) );
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
	UE_LOG( LogTemp, Log, TEXT( "USoundEffectManager: playing 3D sound: %s" ), *entry.Sound->GetName() );
}
