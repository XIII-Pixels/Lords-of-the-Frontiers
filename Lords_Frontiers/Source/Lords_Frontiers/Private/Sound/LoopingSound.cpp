// Fill out your copyright notice in the Description page of Project Settings.

#include "Sound/LoopingSound.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void ULoopingSound::Initialize( const FLoopingSoundConfig* soundConfig )
{
	if ( !soundConfig || !IsValid( soundConfig->Sound ) )
	{
		return;
	}

	AudioComponent_ = UGameplayStatics::CreateSound2D(
	    GetWorld(), soundConfig->Sound, soundConfig->Volume, soundConfig->Pitch, 0.0f, nullptr, true, false
	);

	if ( AudioComponent_ )
	{
		SoundConfig_ = soundConfig;
	}
}

void ULoopingSound::Play()
{
	if ( !IsValid( AudioComponent_ ) || !SoundConfig_ || !IsValid( SoundConfig_->Sound ) )
	{
		return;
	}

	// Initial delayed start
	const float initialDelay = SoundConfig_->DelayBeforeStartRandomized();
	if ( initialDelay <= 0 )
	{
		StartPlayback( true );
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer( StartTimerHandle_ );

		FTimerDelegate delegate;
		delegate.BindWeakLambda( this, [this]() { StartPlayback( true ); } );
		GetWorld()->GetTimerManager().SetTimer( StartTimerHandle_, delegate, initialDelay, false );
	}
}

void ULoopingSound::StartPlayback( bool initial )
{
	if ( !IsValid( AudioComponent_ ) || !SoundConfig_ || !IsValid( SoundConfig_->Sound ) )
	{
		return;
	}

	float fadeIn;
	if ( initial )
	{
		fadeIn = SoundConfig_->InitialFadeInRandomized();
		UE_LOG(
		    LogTemp, Log, TEXT( "ULoopingSound: start playing looping sound: %s" ), *SoundConfig_->Sound->GetName()
		);
	}
	else
	{
		fadeIn = SoundConfig_->FadeInRandomized();
		UE_LOG(
		    LogTemp, Log, TEXT( "ULoopingSound: restart playing looping sound: %s" ), *SoundConfig_->Sound->GetName()
		);
	}

	const float startTime = SoundConfig_->StartTimeRandomized();
	const float fadeOut = SoundConfig_->FadeOutRandomized();
	const float repeatDelay = SoundConfig_->RepeatDelayRandomized();

	float duration = FMath::Max( 0, SoundConfig_->Sound->GetDuration() );
	if ( AudioComponent_->PitchMultiplier > 0.0f )
	{
		duration /= AudioComponent_->PitchMultiplier;
	}
	duration -= startTime;

	GetWorld()->GetTimerManager().ClearTimer( FadeTimerHandle_ );
	GetWorld()->GetTimerManager().ClearTimer( ReplayTimerHandle_ );

	// Play with fade in
	AudioComponent_->FadeIn( fadeIn, 1.0f, startTime );

	// Schedule fade out
	if ( fadeOut > 0 )
	{
		const float fadeOutStart = FMath::Max( 0.0f, duration - fadeOut );

		FTimerDelegate delegate;
		delegate.BindWeakLambda(
		    this,
		    [this, fadeOut]()
		    {
			    if ( IsValid( AudioComponent_ ) )
			    {
				    AudioComponent_->FadeOut( fadeOut, 0.0f );
			    }
		    }
		);

		GetWorld()->GetTimerManager().SetTimer( FadeTimerHandle_, delegate, fadeOutStart, false );
	}

	// Schedule replay
	const float timeBeforeReplay = duration + repeatDelay;

	FTimerDelegate delegate;
	delegate.BindWeakLambda( this, [this]() { StartPlayback(); } );
	GetWorld()->GetTimerManager().SetTimer( ReplayTimerHandle_, delegate, timeBeforeReplay, false );
}

void ULoopingSound::ClearTimers()
{
	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject( this );
	}
}

void ULoopingSound::Stop( bool instant )
{
	ClearTimers();

	float fadeOut = 0.0f;
	if ( !instant && SoundConfig_ )
	{
		fadeOut = SoundConfig_->TransitionFadeOutRandomized();
	}

	if ( IsValid( AudioComponent_ ) )
	{
		AudioComponent_->FadeOut( fadeOut, 0.0f );

		UE_LOG( LogTemp, Log, TEXT( "ULoopingSound: sound is stopped: %s" ), *AudioComponent_->Sound->GetName() );
	}
}

void ULoopingSound::BeginDestroy()
{
	ClearTimers();

	if ( IsValid( AudioComponent_ ) )
	{
		AudioComponent_->bAutoDestroy = true;
		UE_LOG(
		    LogTemp, Log,
		    TEXT( "ULoopingSound: ULoopingSound is destroyed; sound has been marked as destroyed on stop: %s" ),
		    *AudioComponent_->Sound->GetName()
		);
	}
	Stop();

	Super::BeginDestroy();
}
