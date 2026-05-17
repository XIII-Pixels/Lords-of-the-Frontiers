#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "AudioSettingsSubsystem.generated.h"

UENUM( BlueprintType )
enum class EAudioCategory : uint8
{
	Music    UMETA( DisplayName = "Music" ),
	Ambient  UMETA( DisplayName = "Ambient" ),
	Effects  UMETA( DisplayName = "Effects" ),
	UI       UMETA( DisplayName = "UI" ),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnAudioVolumeChanged, EAudioCategory, Category, float, Volume );

/**
 * Stores per-category audio volumes and persists them between sessions.
 * Default value for every category is 0.3.
 */
UCLASS()
class LORDS_FRONTIERS_API UAudioSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr float DefaultCategoryVolume = 0.3f;

	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	UFUNCTION( BlueprintCallable, Category = "Audio|Settings" )
	float GetVolume( EAudioCategory category ) const;

	UFUNCTION( BlueprintCallable, Category = "Audio|Settings" )
	void SetVolume( EAudioCategory category, float volume );

	UFUNCTION( BlueprintCallable, Category = "Audio|Settings" )
	void SaveSettings() const;

	UPROPERTY( BlueprintAssignable, Category = "Audio|Settings" )
	FOnAudioVolumeChanged OnVolumeChanged;

private:
	void LoadSettings();

	UPROPERTY()
	float MusicVolume_ = DefaultCategoryVolume;

	UPROPERTY()
	float AmbientVolume_ = DefaultCategoryVolume;

	UPROPERTY()
	float EffectsVolume_ = DefaultCategoryVolume;

	UPROPERTY()
	float UIVolume_ = DefaultCategoryVolume;

	static const FString SaveSlotName;
};
