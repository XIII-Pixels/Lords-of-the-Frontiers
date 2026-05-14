// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Sound/MusicAmbientManager.h"

#include "Core/DefaultGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/Data/MusicDataAsset.h"

void UMusicAmbientManager::PlayMusic( const FContinuousSound* sound )
{
	if ( Music_ && Music_->IsPlaying() )
	{
		if ( Music_->GetSound() == sound->Sound )
		{
			return;
		}
		Music_->Stop();
		Music_->DestroyComponent();
	}

	if ( sound )
	{
		Music_ = CreateAndPlay( sound );
	}
}

void UMusicAmbientManager::PlayAmbient( const FContinuousSound* sound )
{
	if ( !sound )
	{
		return;
	}

	if ( UAudioComponent* audio = CreateAndPlay( sound ) )
	{
		AmbientSounds_.Add( audio );
	}
}

UAudioComponent* UMusicAmbientManager::CreateAndPlay( const FContinuousSound* sound ) const
{
	if ( !sound )
	{
		return nullptr;
	}

	if ( UAudioComponent* audio = UGameplayStatics::CreateSound2D(
	         GetWorld(), sound->Sound, sound->Volume, sound->Pitch, 0.0f, nullptr, true, false
	     ) )
	{
		PlayContinuousSound( audio, sound );

		return audio;
	}
	return nullptr;
}

void UMusicAmbientManager::PlayContinuousSound( UAudioComponent* audioComponent, const FContinuousSound* sound ) const
{
	if ( !audioComponent || !sound || !sound->Sound )
	{
		return;
	}

	// Initial delayed start
	const float initialDelay = sound->DelayBeforeStartRandomized();
	if ( initialDelay <= 0 )
	{
		StartPlayback( audioComponent, sound, true );
	}
	else
	{
		FTimerHandle startTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
		    startTimerHandle, [this, audioComponent = TWeakObjectPtr( audioComponent ), sound]()
		    { StartPlayback( audioComponent.Get(), sound, true ); }, initialDelay, false
		);
	}
}

void UMusicAmbientManager::StartPlayback( UAudioComponent* audioComponent, const FContinuousSound* sound, bool initial )
    const
{
	if ( !audioComponent || !sound || !sound->Sound )
	{
		return;
	}

	float fadeIn;
	if ( initial )
	{
		fadeIn = sound->InitialFadeInRandomized();
		UE_LOG(
		    LogTemp, Log, TEXT( "UMusicAmbientManager: Start playing looping sound: %s" ), *sound->Sound->GetName()
		);
	}
	else
	{
		fadeIn = sound->FadeInRandomized();
		UE_LOG( LogTemp, Log, TEXT( "UMusicAmbientManager: Replaying looping sound: %s" ), *sound->Sound->GetName() );
	}

	// Play with fade in
	const float startTime = sound->StartTimeRandomized();
	audioComponent->FadeIn( fadeIn, 1.0f, startTime );

	// Schedule fade out
	const float fadeOut = sound->FadeOutRandomized();
	if ( fadeOut > 0 )
	{
		const float duration = sound->Sound->GetDuration() - startTime;
		const float fadeOutStart = FMath::Max( 0.0f, duration - fadeOut );

		FTimerHandle fadeTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
		    fadeTimerHandle,
		    [audio = TWeakObjectPtr( audioComponent ), fadeOut]()
		    {
			    if ( audio.IsValid() )
			    {
				    audio->FadeOut( fadeOut, 0.0f );
				    UE_LOG(
				        LogTemp, Log, TEXT( "UMusicAmbientManager: Start fading out: %s" ), *audio->Sound->GetName()
				    );
			    }
		    },
		    fadeOutStart, false
		);
	}

	// Schedule replay
	const float repeatDelay = sound->RepeatDelayRandomized();
	const float timeBeforeReplay = sound->Sound->GetDuration() - startTime + repeatDelay;
	FTimerHandle replayTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
	    replayTimerHandle, [this, audioComponent, sound]() { StartPlayback( audioComponent, sound ); },
	    timeBeforeReplay, false
	);
}

void UMusicAmbientManager::PlayMainMenuMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->MainMenuMusic() );
	}
}

void UMusicAmbientManager::PlayWinBattleMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->WinBattleMusic() );
	}
}

void UMusicAmbientManager::PlayWinGameMusic()
{
	if ( MusicDataAsset_.IsValid() )
	{
		PlayMusic( &MusicDataAsset_->WinGameMusic() );
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
