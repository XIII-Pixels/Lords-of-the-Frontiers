#pragma once

#include "AudioEvent.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "CoreMinimal.h"

#include "SoundEffectManager.generated.h"

struct FSoundEntry;
class USoundDataAsset;
class UAudioComponent;
class USoundAttenuation;

/** (Gregory-hub) */
UCLASS()
class LORDS_FRONTIERS_API USoundEffectManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	// Use UnregisterObject if using RegisterObject
	void RegisterObject( UObject* object );
	void UnregisterObject( UObject* object );

	/** Plays a 2D (non-positional) sound by tag for systems that are not IAudioEventSource,
	 *  e.g. a reward cue. volumeScale is an extra multiplier on top of the sound entry volume. */
	void PlaySound2D( const FGameplayTag& tag, float volumeScale = 1.0f );

private:
	UFUNCTION()
	void HandleAudioEvent( FAudioEvent event );

	TWeakObjectPtr<UAudioComponent> AcquireAudioComponent();
	void ReleaseAudioComponent( TWeakObjectPtr<UAudioComponent> component );

	void OnSoundFinished( TWeakObjectPtr<UAudioComponent> component );

	void Play2D( const FSoundEntry& entry, const FGameplayTag& tag, float volumeScale = 1.0f );
	void Play3D( const FSoundEntry& entry, const FGameplayTag& tag, const FVector& worldLocation );

	// Pooled audio components are created with the current world as outer. The pools live in
	// the GameInstance, so they must drop the components of a world that is being torn down:
	// otherwise they kept the old world alive and were reused in the next level.
	void HandleWorldCleanup( UWorld* world, bool bSessionEnded, bool bCleanupResources );

	FDelegateHandle WorldCleanupHandle_;

	UPROPERTY()
	TObjectPtr<USoundDataAsset> SoundData_;

	UPROPERTY()
	TObjectPtr<USoundAttenuation> DefaultAttenuation_;

	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> FreeComponentsPool_;

	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> ComponentsInUsePool_;
};
