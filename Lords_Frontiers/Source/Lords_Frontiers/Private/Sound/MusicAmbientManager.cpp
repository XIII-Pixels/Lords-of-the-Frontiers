// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Sound/MusicAmbientManager.h"

#include "Core/DefaultGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/Data/AmbientDataAsset.h"
#include "Sound/Data/MusicDataAsset.h"
#include "Sound/LoopingSound.h"

ULoopingSound* UMusicAmbientManager::PlayMusic( const FLoopingSoundConfig* sound )
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
		Music_ = CreateAndPlay( sound );
	}
	return Music_;
}

ULoopingSound* UMusicAmbientManager::PlayAmbient( const FLoopingSoundConfig* sound )
{
	if ( !sound )
	{
		return nullptr;
	}

	if ( ULoopingSound* audio = CreateAndPlay( sound ) )
	{
		AmbientSounds_.Add( audio );
		return audio;
	}
	return nullptr;
}

ULoopingSound* UMusicAmbientManager::CreateAndPlay( const FLoopingSoundConfig* sound )
{
	if ( !sound )
	{
		return nullptr;
	}

	if ( auto* audio = NewObject<ULoopingSound>( this ) )
	{
		audio->Initialize( sound );
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
		PlayMusic( &MusicDataAsset_->WinMusic() );
	}
}

void UMusicAmbientManager::PlayLoseGameMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->LoseMusic() );
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
			PlayMusic( &musicForLevel->Battle );
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
		for ( const FLoopingSoundConfig& ambient : ambientForLevel->AmbientEntries )
		{
			PlayAmbient( &ambient );
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

void UMusicAmbientManager::StopMusic()
{
	if ( Music_ )
	{
		Music_->Stop();
		Music_ = nullptr;
	}
}

void UMusicAmbientManager::StopAllAmbient()
{
	for ( ULoopingSound* audio : AmbientSounds_ )
	{
		if ( IsValid( audio ) )
		{
			audio->Stop();
		}
	}
	AmbientSounds_.Empty();

	UE_LOG( LogTemp, Log, TEXT( "UMusicAmbientManager: All ambient is stopped" ) );
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
}

void UMusicAmbientManager::Deinitialize()
{
	StopMusic();
	StopAllAmbient();

	Super::Deinitialize();
}
