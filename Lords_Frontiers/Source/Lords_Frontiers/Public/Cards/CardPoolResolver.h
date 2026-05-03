#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"

class UCardDataAsset;

/**
 * FCardRarityBucket
 *
 * One rarity tier passed into the resolver. Cards in this bucket should already
 * be filtered for unlock status. Per-card weights and exclusion rules are still
 * applied internally by FCardPoolResolver::Resolve.
 */
struct LORDS_FRONTIERS_API FCardRarityBucket
{
	ECardRarity Rarity         = ECardRarity::Common;
	float       RarityWeight   = 0.f;
	TArray<UCardDataAsset*> Cards;
};

class LORDS_FRONTIERS_API FCardPoolResolver
{
public:
	static TArray<UCardDataAsset*> Resolve(
		const TArray<FCardRarityBucket>& buckets,
		const TArray<FAppliedCardRecord>& history,
		int32 countToOffer,
		int32 maxCardsPerRarity,
		int32 maxStacksForWeightInfluence = MAX_int32,
		const TSet<UCardDataAsset*>& weightReducedCards = TSet<UCardDataAsset*>(),
		float weightReductionMultiplier = 1.f );
};
