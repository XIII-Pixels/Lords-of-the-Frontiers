// Fill out your copyright notice in the Description page of Project Settings.

#include "Sound/MusicAmbientManager.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UMusicAmbientManager::Play( const FMusicConfig& config )
{
	if ( !config.Sound )
	{
		return;
	}

	if ( !config.bIndependent )
	{
		if ( Music_ && Music_->IsPlaying() )
		{
			Music_->Stop();
		}

		Music_ = UGameplayStatics::SpawnSound2D( GetWorld(), config.Sound, 1, 1, 0, nullptr, true, false );
	}
	else
	{
		UAudioComponent* audio =
		    UGameplayStatics::SpawnSound2D( GetWorld(), config.Sound, 1, 1, 0, nullptr, true, false );
		if ( audio )
		{
			AmbientSounds_.Add( audio );
		}
	}
}

void UMusicAmbientManager::PlayMainMenuMusic()
{
}

void UMusicAmbientManager::PlayBuildingMusic()
{
}

void UMusicAmbientManager::PlayCombatMusic()
{
}

void UMusicAmbientManager::PlayWinGameMusic()
{
}

void UMusicAmbientManager::PlayLoseGameMusic()
{
}

void UMusicAmbientManager::StopMusic()
{
	StopAudio( Music_ );
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
}

void UMusicAmbientManager::StopAudio( UAudioComponent* audio )
{
	if ( audio )
	{
		if ( audio == Music_ )
		{
			Music_ = nullptr;
		}
		else
		{
			AmbientSounds_.Remove( audio );
		}
		audio->DestroyComponent();
	}
}

void UMusicAmbientManager::Deinitialize()
{
	if ( Music_ )
	{
		Music_->DestroyComponent();
		Music_ = nullptr;
	}

	for ( UAudioComponent* audio : AmbientSounds_ )
	{
		if ( audio )
		{
			audio->DestroyComponent();
		}
	}
	AmbientSounds_.Empty();

	Super::Deinitialize();
}
