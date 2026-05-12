// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Sound/MusicAmbientManager.h"

#include "Core/DefaultGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/MusicDataAsset.h"

void UMusicAmbientManager::PlayMusic( USoundBase* sound )
{
	if ( Music_ && Music_->IsPlaying() )
	{
		if ( Music_->GetSound() == sound )
		{
			return;
		}
		Music_->Stop();
		Music_->DestroyComponent();
	}

	if ( sound )
	{
		Music_ = PlayLoopingSound( sound );
	}
}

void UMusicAmbientManager::PlayAmbient( USoundBase* sound )
{
	if ( !sound )
	{
		return;
	}

	if ( UAudioComponent* audio = PlayLoopingSound( sound ) )
	{
		AmbientSounds_.Add( audio );
	}
}

UAudioComponent* UMusicAmbientManager::PlayLoopingSound( USoundBase* sound ) const
{
	if ( !sound )
	{
		return nullptr;
	}

	if ( UAudioComponent* audio = UGameplayStatics::SpawnSound2D( GetWorld(), sound, 1, 1, 0, nullptr, true, false ) )
	{
		audio->OnAudioFinishedNative.AddWeakLambda(
		    this,
		    [this, audio = TWeakObjectPtr<UAudioComponent>( audio )]( UAudioComponent* )
		    {
			    if ( audio.IsValid() )
			    {
				    audio->Play();
			    }
		    }
		);
		audio->FadeIn( 0.5f, 1.0f );

		UE_LOG( LogTemp, Log, TEXT( "UMusicAmbientManager: Playing looping sound: %s" ), *sound->GetName() );

		return audio;
	}
	return nullptr;
}

void UMusicAmbientManager::PlayMainMenuMusic()
{
	USoundBase* sound = nullptr;
	if ( MusicDataAsset_.IsValid() )
	{
		sound = MusicDataAsset_->MainMenuMusic();
	}
	PlayMusic( sound );
}

void UMusicAmbientManager::PlayWinBattleMusic()
{
	USoundBase* sound = nullptr;
	if ( MusicDataAsset_.IsValid() )
	{
		sound = MusicDataAsset_->WinBattleMusic();
	}
	PlayMusic( sound );
}

void UMusicAmbientManager::PlayWinGameMusic()
{
	USoundBase* sound = nullptr;
	if ( MusicDataAsset_.IsValid() )
	{
		sound = MusicDataAsset_->WinGameMusic();
	}
	PlayMusic( sound );
}

void UMusicAmbientManager::PlayLoseGameMusic()
{
	USoundBase* sound = nullptr;
	if ( MusicDataAsset_.IsValid() )
	{
		sound = MusicDataAsset_->LoseMusic();
	}
	PlayMusic( sound );
}

void UMusicAmbientManager::PlayCurrentLevelBuildingMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		if ( const FMusicForLevel* musicForLevel =
		         MusicDataAsset_->MusicForLevel( TSoftObjectPtr<UWorld>( GetWorld() ) ) )
		{
			if ( USoundBase* sound = musicForLevel->Building )
			{
				PlayMusic( sound );
				return;
			}
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
			if ( USoundBase* sound = musicForLevel->Battle )
			{
				PlayMusic( sound );
				return;
			}
		}
	}
	StopMusic();
}

void UMusicAmbientManager::PlayCurrentLevelAmbient()
{
}

void UMusicAmbientManager::StopMusic()
{
	if ( Music_ )
	{
		UE_LOG( LogTemp, Log, TEXT( "UMusicAmbientManager: Music is stopped: %s" ), *Music_->GetName() );

		Music_->Stop();
		Music_->DestroyComponent();
		Music_ = nullptr;
	}
}

void UMusicAmbientManager::StopAllAmbient()
{
	for ( UAudioComponent* audio : AmbientSounds_ )
	{
		if ( IsValid( audio ) )
		{
			audio->DestroyComponent();
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
	if ( Music_ )
	{
		Music_->Stop();
		Music_->DestroyComponent();
		Music_ = nullptr;
	}

	for ( UAudioComponent* audio : AmbientSounds_ )
	{
		if ( audio )
		{
			audio->Stop();
			audio->DestroyComponent();
		}
	}
	AmbientSounds_.Empty();

	Super::Deinitialize();
}
