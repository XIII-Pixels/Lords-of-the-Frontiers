#include "Sound/AudioSettingsSubsystem.h"

#include "Sound/AudioSettingsSave.h"

#include "Kismet/GameplayStatics.h"

const FString UAudioSettingsSubsystem::SaveSlotName = TEXT( "AudioSettings" );

void UAudioSettingsSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );
	LoadSettings();
}

void UAudioSettingsSubsystem::Deinitialize()
{
	SaveSettings();
	Super::Deinitialize();
}

float UAudioSettingsSubsystem::GetVolume( EAudioCategory category ) const
{
	switch ( category )
	{
		case EAudioCategory::Music:   return MusicVolume_;
		case EAudioCategory::Ambient: return AmbientVolume_;
		case EAudioCategory::Effects: return EffectsVolume_;
		case EAudioCategory::UI:      return UIVolume_;
	}
	return DefaultCategoryVolume;
}

void UAudioSettingsSubsystem::SetVolume( EAudioCategory category, float volume )
{
	volume = FMath::Clamp( volume, 0.0f, 1.0f );

	float* target = nullptr;
	switch ( category )
	{
		case EAudioCategory::Music:   target = &MusicVolume_;   break;
		case EAudioCategory::Ambient: target = &AmbientVolume_; break;
		case EAudioCategory::Effects: target = &EffectsVolume_; break;
		case EAudioCategory::UI:      target = &UIVolume_;      break;
	}

	if ( !target || FMath::IsNearlyEqual( *target, volume ) )
	{
		return;
	}

	*target = volume;
	OnVolumeChanged.Broadcast( category, volume );
}

void UAudioSettingsSubsystem::SaveSettings() const
{
	UAudioSettingsSave* save =
	    Cast<UAudioSettingsSave>( UGameplayStatics::CreateSaveGameObject( UAudioSettingsSave::StaticClass() ) );
	if ( !save )
	{
		return;
	}

	save->MusicVolume   = MusicVolume_;
	save->AmbientVolume = AmbientVolume_;
	save->EffectsVolume = EffectsVolume_;
	save->UIVolume      = UIVolume_;

	UGameplayStatics::SaveGameToSlot( save, SaveSlotName, 0 );
}

void UAudioSettingsSubsystem::LoadSettings()
{
	if ( !UGameplayStatics::DoesSaveGameExist( SaveSlotName, 0 ) )
	{
		return;
	}

	UAudioSettingsSave* save = Cast<UAudioSettingsSave>( UGameplayStatics::LoadGameFromSlot( SaveSlotName, 0 ) );
	if ( !save )
	{
		return;
	}

	MusicVolume_   = FMath::Clamp( save->MusicVolume,   0.0f, 1.0f );
	AmbientVolume_ = FMath::Clamp( save->AmbientVolume, 0.0f, 1.0f );
	EffectsVolume_ = FMath::Clamp( save->EffectsVolume, 0.0f, 1.0f );
	UIVolume_      = FMath::Clamp( save->UIVolume,      0.0f, 1.0f );
}
