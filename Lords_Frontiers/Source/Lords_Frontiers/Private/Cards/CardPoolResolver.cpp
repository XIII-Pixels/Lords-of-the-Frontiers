#include "Cards/CardPoolResolver.h"

#include "Cards/CardDataAsset.h"

#include "Math/UnrealMathUtility.h"

namespace
{
	struct FWorkingBucket
	{
		ECardRarity Rarity       = ECardRarity::Common;
		float       RarityWeight = 0.f;
		TMap<UCardDataAsset*, float> Weights;
	};

	int32 PickIndexByWeight( const TArray<float>& weights, float totalWeight )
	{
		const float roll = FMath::FRand() * totalWeight;
		float running    = 0.f;
		for ( int32 i = 0; i < weights.Num(); ++i )
		{
			running += weights[i];
			if ( roll <= running )
			{
				return i;
			}
		}
		return weights.Num() - 1;
	}

	UCardDataAsset* PickCardFromBucket( FWorkingBucket& bucket )
	{
		float totalWeight = 0.f;
		TArray<UCardDataAsset*> keys;
		TArray<float> values;
		keys.Reserve( bucket.Weights.Num() );
		values.Reserve( bucket.Weights.Num() );

		for ( const auto& pair : bucket.Weights )
		{
			if ( pair.Key && pair.Value > 0.f )
			{
				keys.Add( pair.Key );
				values.Add( pair.Value );
				totalWeight += pair.Value;
			}
		}

		if ( keys.Num() == 0 || totalWeight <= 0.f )
		{
			return nullptr;
		}

		const int32 picked = PickIndexByWeight( values, totalWeight );
		UCardDataAsset* card = keys[picked];
		bucket.Weights.Remove( card );
		return card;
	}
}

TArray<UCardDataAsset*> FCardPoolResolver::Resolve(
	const TArray<FCardRarityBucket>& buckets,
	const TArray<FAppliedCardRecord>& history,
	int32 countToOffer,
	int32 maxCardsPerRarity,
	int32 maxStacksForWeightInfluence,
	const TSet<UCardDataAsset*>& weightReducedCards,
	float weightReductionMultiplier )
{
	TArray<UCardDataAsset*> result;

	if ( buckets.Num() == 0 || countToOffer <= 0 || maxCardsPerRarity <= 0 )
	{
		return result;
	}

	TSet<UCardDataAsset*> excludedByOtherCards;
	for ( const FAppliedCardRecord& record : history )
	{
		UCardDataAsset* source = record.Card;
		if ( !source )
		{
			continue;
		}

		for ( const TObjectPtr<UCardDataAsset>& exc : source->ExcludedCards )
		{
			if ( exc )
			{
				excludedByOtherCards.Add( exc );
			}
		}
	}

	auto IsCardAlreadyApplied = [&]( UCardDataAsset* card )
	{
		for ( const FAppliedCardRecord& record : history )
		{
			if ( record.Card == card )
			{
				return true;
			}
		}
		return false;
	};

	TArray<FWorkingBucket> workingBuckets;
	workingBuckets.Reserve( buckets.Num() );

	const float clampedReductionMultiplier = FMath::Max( 0.f, weightReductionMultiplier );
	const bool bApplyReductionMultiplier   = weightReducedCards.Num() > 0 && weightReductionMultiplier < 1.f;

	for ( const FCardRarityBucket& src : buckets )
	{
		if ( src.RarityWeight <= 0.f || src.Cards.Num() == 0 )
		{
			continue;
		}

		FWorkingBucket wb;
		wb.Rarity       = src.Rarity;
		wb.RarityWeight = src.RarityWeight;
		wb.Weights.Reserve( src.Cards.Num() );

		for ( UCardDataAsset* card : src.Cards )
		{
			if ( !card )
			{
				continue;
			}
			if ( excludedByOtherCards.Contains( card ) )
			{
				continue;
			}
			if ( !card->bCanStack && IsCardAlreadyApplied( card ) )
			{
				continue;
			}

			float weight = card->BaseWeight;
			if ( bApplyReductionMultiplier && weightReducedCards.Contains( card ) )
			{
				weight *= clampedReductionMultiplier;
			}

			wb.Weights.Add( card, weight );
		}

		if ( wb.Weights.Num() > 0 )
		{
			workingBuckets.Add( MoveTemp( wb ) );
		}
	}

	if ( workingBuckets.Num() == 0 )
	{
		return result;
	}

	auto applyMultiplierMap = [&]( const TMap<TObjectPtr<UCardDataAsset>, float>& map, int32 effectiveStacks )
	{
		for ( const auto& pair : map )
		{
			UCardDataAsset* target = pair.Key;
			if ( !target )
			{
				continue;
			}
			for ( FWorkingBucket& wb : workingBuckets )
			{
				if ( float* weight = wb.Weights.Find( target ) )
				{
					for ( int32 i = 0; i < effectiveStacks; ++i )
					{
						*weight *= pair.Value;
					}
				}
			}
		}
	};

	auto applyCategoryMultiplierMap = [&]( const TMap<ECardCategory, float>& map, int32 effectiveStacks )
	{
		for ( const auto& pair : map )
		{
			const ECardCategory category = pair.Key;
			const float multiplier       = pair.Value;
			for ( FWorkingBucket& wb : workingBuckets )
			{
				for ( auto& weightPair : wb.Weights )
				{
					if ( weightPair.Key && weightPair.Key->Category == category )
					{
						for ( int32 i = 0; i < effectiveStacks; ++i )
						{
							weightPair.Value *= multiplier;
						}
					}
				}
			}
		}
	};

	auto applyTagMultiplierMap = [&]( const TMap<FName, float>& map, int32 effectiveStacks )
	{
		for ( const auto& pair : map )
		{
			const FName tag        = pair.Key;
			const float multiplier = pair.Value;
			if ( tag.IsNone() )
			{
				continue;
			}
			for ( FWorkingBucket& wb : workingBuckets )
			{
				for ( auto& weightPair : wb.Weights )
				{
					if ( weightPair.Key && weightPair.Key->Tags.Contains( tag ) )
					{
						for ( int32 i = 0; i < effectiveStacks; ++i )
						{
							weightPair.Value *= multiplier;
						}
					}
				}
			}
		}
	};

	for ( const FAppliedCardRecord& record : history )
	{
		UCardDataAsset* source = record.Card;
		if ( !source )
		{
			continue;
		}

		const int32 rawStacks       = FMath::Max( 1, record.StackCount );
		const int32 effectiveStacks = FMath::Min( rawStacks, FMath::Max( 1, maxStacksForWeightInfluence ) );

		applyMultiplierMap( source->WeightMultipliers_Up, effectiveStacks );
		applyMultiplierMap( source->WeightMultipliers_Down, effectiveStacks );
		applyCategoryMultiplierMap( source->CategoryWeightMultipliers_Up, effectiveStacks );
		applyCategoryMultiplierMap( source->CategoryWeightMultipliers_Down, effectiveStacks );
		applyTagMultiplierMap( source->TagWeightMultipliers_Up, effectiveStacks );
		applyTagMultiplierMap( source->TagWeightMultipliers_Down, effectiveStacks );
	}

	TMap<ECardRarity, int32> perRarityCount;

	while ( result.Num() < countToOffer )
	{
		TArray<int32> candidateBucketIndices;
		TArray<float> candidateBucketWeights;
		float candidateTotal = 0.f;

		for ( int32 i = 0; i < workingBuckets.Num(); ++i )
		{
			FWorkingBucket& wb = workingBuckets[i];
			const int32 already = perRarityCount.FindRef( wb.Rarity );
			if ( already >= maxCardsPerRarity )
			{
				continue;
			}

			float bucketAvailableWeight = 0.f;
			for ( const auto& pair : wb.Weights )
			{
				if ( pair.Value > 0.f )
				{
					bucketAvailableWeight += pair.Value;
				}
			}
			if ( bucketAvailableWeight <= 0.f )
			{
				continue;
			}

			candidateBucketIndices.Add( i );
			candidateBucketWeights.Add( wb.RarityWeight );
			candidateTotal += wb.RarityWeight;
		}

		if ( candidateBucketIndices.Num() == 0 || candidateTotal <= 0.f )
		{
			break;
		}

		const int32 pickedCandidate = PickIndexByWeight( candidateBucketWeights, candidateTotal );
		const int32 bucketIdx       = candidateBucketIndices[pickedCandidate];

		UCardDataAsset* picked = PickCardFromBucket( workingBuckets[bucketIdx] );
		if ( !picked )
		{
			continue;
		}

		result.Add( picked );
		perRarityCount.FindOrAdd( workingBuckets[bucketIdx].Rarity )++;
	}

	return result;
}
