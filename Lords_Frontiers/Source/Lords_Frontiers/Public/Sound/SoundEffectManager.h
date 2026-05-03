#pragma once

#include "AudioEvent.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "CoreMinimal.h"
#include "SoundDataAsset.h"

#include "SoundEffectManager.generated.h"

class UAudioComponent;
class USoundAttenuation;

/** (Gregory-hub) */
UCLASS()
class LORDS_FRONTIERS_API USoundEffectManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	virtual void Deinitialize() override;

	void RegisterActor( AActor* actor );
	void UnregisterActor( AActor* actor );

private:
	UFUNCTION()
	void HandleAudioEvent( FAudioEvent event );

	TWeakObjectPtr<UAudioComponent> AcquireAudioComponent();
	void ReleaseAudioComponent( TWeakObjectPtr<UAudioComponent> component );

	void OnSoundFinished( TWeakObjectPtr<UAudioComponent> component );

	void Play2D( const FSoundEntry& entry );
	void Play3D( const FSoundEntry& entry, const FVector& worldLocation );

	UPROPERTY()
	TObjectPtr<USoundDataAsset> SoundData_;

	UPROPERTY()
	TObjectPtr<USoundAttenuation> DefaultAttenuation_;

	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> FreeComponentsPool_;

	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> ComponentsInUsePool_;
};
