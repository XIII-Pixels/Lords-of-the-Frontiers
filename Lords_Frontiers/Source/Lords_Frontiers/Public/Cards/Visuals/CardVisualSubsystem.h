#pragma once

#include "Cards/Visuals/CardVisualTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "CardVisualSubsystem.generated.h"

class ACardFeedbackPopup;
class ACardIconStrip;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class USkeletalMeshComponent;
class UTexture2D;

USTRUCT()
struct FCardStickyIconSlot
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<ACardIconStrip> Strip;

	UPROPERTY()
	int32 SlotId = INDEX_NONE;
};

USTRUCT()
struct FCardStickyRecord
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Host;

	UPROPERTY()
	TArray<FCardStickyIconSlot> IconSlots;

	UPROPERTY()
	TArray<TWeakObjectPtr<UNiagaraComponent>> NiagaraComponents;

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> OverlayMesh;

	UPROPERTY()
	TWeakObjectPtr<UMaterialInterface> OverlayPrevMaterial;

	UPROPERTY()
	bool bOverlayApplied = false;

	UPROPERTY()
	int32 Id = INDEX_NONE;

	UPROPERTY()
	TArray<FTimerHandle> PendingTimers;
};

USTRUCT()
struct FCardVisualDeferred
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Host;

	UPROPERTY()
	TWeakObjectPtr<AActor> Target;

	UPROPERTY()
	FCardVisualConfig Config;

	UPROPERTY()
	int32 StickyId = INDEX_NONE;

	FTimerHandle TimerHandle;

	bool bDoIcon = false;
	bool bDoNiagara = false;
};

UCLASS()
class LORDS_FRONTIERS_API UCardVisualSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCardVisualSubsystem* Get( const UObject* worldContextObject );

	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	virtual void Tick( float deltaTime ) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}
	virtual bool IsTickable() const override;

	void PlayOneShot( const FCardVisualConfig& config, AActor* owner, AActor* target );

	FCardVisualHandle BeginSticky( const FCardVisualConfig& config, AActor* owner, AActor* target );

	void EndSticky( FCardVisualHandle handle );

	UFUNCTION( BlueprintCallable, Category = "Card|Visuals" )
	void EndAllSticky();

	UFUNCTION( BlueprintCallable, Category = "Card|Visuals" )
	void SetPopupClassOverride( TSubclassOf<ACardFeedbackPopup> popupClass );

	UFUNCTION( BlueprintCallable, Category = "Card|Visuals" )
	void PrewarmIconPool( int32 count );

private:
	ACardFeedbackPopup* AcquireIconActor();
	void ReleaseIconActor( ACardFeedbackPopup* actor );
	TSubclassOf<ACardFeedbackPopup> ResolvePopupClass() const;

	ACardIconStrip* GetOrCreateStrip( AActor* host );
	void ReleaseStrip( ACardIconStrip* strip );
	TSubclassOf<ACardIconStrip> ResolveStripClass() const;

	void PlayIcon( const FCardIconSpec& spec, AActor* owner, AActor* target, bool bSticky, int32 stickyId );
	void PlayNiagara( const FCardNiagaraSpec& spec, AActor* owner, AActor* target, bool bSticky, int32 stickyId );
	void PlayOverlay( const FCardOverlayMaterialSpec& spec, AActor* owner, AActor* target, int32 stickyId );

	void SpawnIconNow( const FCardIconSpec& spec, AActor* host, bool bSticky, int32 stickyId );
	void SpawnNiagaraNow( const FCardNiagaraSpec& spec, AActor* host, bool bSticky, int32 stickyId );
	void ApplyOverlayNow( const FCardOverlayMaterialSpec& spec, AActor* host, int32 stickyId );

	static USkeletalMeshComponent* FindSkeletalMeshOn( AActor* host );

	void ResolveTargetHosts(
		ECardVisualTarget target, AActor* owner, AActor* hitTarget, TArray<AActor*>& outHosts ) const;

	int32 AllocateStickyId();
	FCardStickyRecord* FindSticky( int32 id );

	UPROPERTY()
	TArray<TObjectPtr<ACardFeedbackPopup>> FreeIconPool_;

	UPROPERTY()
	TSet<TObjectPtr<ACardFeedbackPopup>> InUseIcons_;

	UPROPERTY()
	TArray<TObjectPtr<ACardIconStrip>> FreeStripPool_;

	UPROPERTY()
	TArray<TObjectPtr<ACardIconStrip>> ActiveStripsList_;

	TMap<TWeakObjectPtr<AActor>, ACardIconStrip*> ActiveStripsByHost_;

	UPROPERTY()
	TArray<FCardStickyRecord> Stickies_;

	UPROPERTY()
	TSubclassOf<ACardFeedbackPopup> PopupClassOverride_;

	UPROPERTY()
	TSubclassOf<ACardIconStrip> StripClassOverride_;

	UPROPERTY()
	TArray<FTimerHandle> PendingTimers_;

	UPROPERTY()
	TArray<TWeakObjectPtr<UNiagaraComponent>> AllSpawnedNiagaras_;

	int32 NextStickyId_ = 1;
};
