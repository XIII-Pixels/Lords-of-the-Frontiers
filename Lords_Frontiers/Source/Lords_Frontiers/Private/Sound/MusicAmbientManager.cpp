// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Sound/MusicAmbientManager.h"

#include "Core/DefaultGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioSettingsSubsystem.h"
#include "Sound/Data/AmbientDataAsset.h"
#include "Sound/Data/MusicDataAsset.h"
#include "Sound/LoopingSound.h"
#include "UObject/UObjectGlobals.h"

ULoopingSound* UMusicAmbientManager::PlayMusic( const FLoopingSoundConfig* sound, float volumeScale )
{
	if ( Music_ )
	{
		if ( IsValid( Music_->GetAudioComponent() ) && Music_->GetAudioComponent()->GetSound() == sound->Sound )
		{
			return nullptr;
		}
		StopMusic();
	}

	if ( sound )
	{
		Music_ = CreateAndPlay( sound, EMusicAmbientKind::Music, volumeScale );
	}
	return Music_;
}

ULoopingSound* UMusicAmbientManager::PlayAmbient( const FLoopingSoundConfig* sound )
{
	if ( !sound )
	{
		return nullptr;
	}

	if ( ULoopingSound* audio = CreateAndPlay( sound, EMusicAmbientKind::Ambient ) )
	{
		AmbientSounds_.Add( audio );
		return audio;
	}
	return nullptr;
}

ULoopingSound* UMusicAmbientManager::CreateAndPlay( const FLoopingSoundConfig* sound, EMusicAmbientKind kind, float volumeScale )
{
	if ( !sound )
	{
		return nullptr;
	}

	if ( auto* audio = NewObject<ULoopingSound>( this ) )
	{
		const EAudioCategory category =
		    kind == EMusicAmbientKind::Music ? EAudioCategory::Music : EAudioCategory::Ambient;
		audio->Initialize( sound, category, volumeScale );
		audio->Play();

		return audio;
	}
	return nullptr;
}

void UMusicAmbientManager::PlayMainMenuMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->MainMenuMusic() );
	}
}

void UMusicAmbientManager::PlayWinGameMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->WinMusic(), MusicDataAsset_->WinLoseMusicVolumeScale() );
	}
}

void UMusicAmbientManager::PlayLoseGameMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->LoseMusic(), MusicDataAsset_->WinLoseMusicVolumeScale() );
	}
}

void UMusicAmbientManager::PlayCurrentLevelBuildingMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		if ( const FMusicForLevel* musicForLevel =
		         MusicDataAsset_->MusicForLevel( TSoftObjectPtr<UWorld>( GetWorld() ) ) )
		{
			PlayMusic( &musicForLevel->Building );
			return;
		}
	}
	StopMusic();
}

void UMusicAmbientManager::PlayCurrentLevelCombatMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		if ( const FMusicForLevel* musicForLevel =
		         MusicDataAsset_->MusicForLevel( TSoftObjectPtr<UWorld>( GetWorld() ) ) )
		{
			PlayMusic( &musicForLevel->Battle, MusicDataAsset_->BattleMusicVolumeScale() );
			return;
		}
	}
	StopMusic();
}

void UMusicAmbientManager::PlayCurrentLevelAmbient()
{
	StopAllAmbient();
	if ( AmbientDataAsset_.IsValid() )
	{
		const FAmbientForLevel* ambientForLevel =
		    AmbientDataAsset_->AmbientForLevel( TSoftObjectPtr<UWorld>( GetWorld() ) );

		if ( ambientForLevel )
		{
			for ( const FLoopingSoundConfig& ambient : ambientForLevel->AmbientEntries )
			{
				PlayAmbient( &ambient );
			}
		}
	}
}

void UMusicAmbientManager::PlayWindAmbient()
{
	if ( !WindAmbientPlaying_.IsValid() && AmbientDataAsset_.IsValid() )
	{
		WindAmbientPlaying_ = PlayAmbient( &AmbientDataAsset_->WindAmbient() );
	}
}

void UMusicAmbientManager::StopWindAmbient()
{
	if ( WindAmbientPlaying_.IsValid() )
	{
		StopAmbient( WindAmbientPlaying_.Get() );
		WindAmbientPlaying_ = nullptr;
	}
}

void UMusicAmbientManager::StopAmbient( ULoopingSound* ambient )
{
	if ( IsValid( ambient ) )
	{
		ambient->Stop();
	}
	AmbientSounds_.Remove( ambient );
}

void UMusicAmbientManager::StopMusic( bool instant )
{
	if ( Music_ )
	{
		Music_->Stop( instant );
		Music_ = nullptr;
	}
}

void UMusicAmbientManager::StopAllAmbient( bool instant )
{
	for ( ULoopingSound* audio : AmbientSounds_ )
	{
		if ( IsValid( audio ) )
		{
			audio->Stop( instant );
		}
	}
	AmbientSounds_.Empty();

	UE_LOG( LogTemp, Log, TEXT( "UMusicAmbientManager: All ambient is stopped" ) );
}

void UMusicAmbientManager::AdjustAmbientVolume( float volumeLevel )
{
	volumeLevel = FMath::Clamp( volumeLevel, 0.001f, 1.0f );

	float min = 0.0f;
	float adjustVolumeDuration = 0.0f;
	EAudioFaderCurve faderCurve = EAudioFaderCurve::Linear;
	if ( AmbientDataAsset_.IsValid() )
	{
		if ( const FAmbientForLevel* ambientForLevel =
		         AmbientDataAsset_->AmbientForLevel( TSoftObjectPtr( GetWorld() ) ) )
		{
			min = FMath::Clamp( ambientForLevel->ZoomMinVolume, 0.001f, 1.0f );
			adjustVolumeDuration = ambientForLevel->AdjustVolumeDuration;
			faderCurve = ambientForLevel->ZoomVolumeFaderCurve;
		}
	}

	// Only the zoom factor goes into the component's fade volume. The per-sound config volume
	// and the Ambient category volume from the settings are already applied through
	// SetVolumeMultiplier in ULoopingSound; multiplying them in here applied them twice and
	// fought with the volume slider.
	const float zoomVolume = FMath::Max( volumeLevel, min );

	for ( ULoopingSound* audio : AmbientSounds_ )
	{
		if ( IsValid( audio ) && audio != WindAmbientPlaying_.Get() && IsValid( audio->GetAudioComponent() ) )
		{
			audio->GetAudioComponent()->AdjustVolume( adjustVolumeDuration, zoomVolume, faderCurve );
		}
	}
}

void UMusicAmbientManager::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );

	if ( const auto* gameInstance = Cast<UDefaultGameInstance>( GetGameInstance() ) )
	{
		MusicDataAsset_ = gameInstance->Music;
		AmbientDataAsset_ = gameInstance->Ambient;
		LevelsDataAsset_ = gameInstance->Levels;
	}

	PreLoadMapHandle_ = FCoreUObjectDelegates::PreLoadMap.AddUObject( this, &UMusicAmbientManager::HandlePreLoadMap );
}

void UMusicAmbientManager::Deinitialize()
{
	if ( PreLoadMapHandle_.IsValid() )
	{
		FCoreUObjectDelegates::PreLoadMap.Remove( PreLoadMapHandle_ );
		PreLoadMapHandle_.Reset();
	}

	StopMusic();
	StopAllAmbient();

	Super::Deinitialize();
}

void UMusicAmbientManager::HandlePreLoadMap( const FString& /*mapName*/ )
{
	// Looping music/ambient are spawned with bPersistAcrossLevelTransition, so they keep playing
	// when the world is swapped. Stop them the moment a new level begins loading; the level being
	// loaded restarts whatever music/ambient it needs from its own GameMode / BeginPlay.
	StopMusic( true );
	StopAllAmbient( true );
}
