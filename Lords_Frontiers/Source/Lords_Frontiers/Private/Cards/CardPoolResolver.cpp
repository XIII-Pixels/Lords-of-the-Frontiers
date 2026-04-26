#include "Cards/CardPoolResolver.h"

#include "Cards/CardDataAsset.h"

#include "Math/UnrealMathUtility.h"

namespace
{
	TArray<UCardDataAsset*> WeightedSample(
		TArray<TPair<UCardDataAsset*, float>>& weighted, int32 count )
	{
		TArray<UCardDataAsset*> result;

		float totalWeight = 0.f;
		for ( const auto& pair : weighted )
		{
			totalWeight += pair.Value;
		}

		while ( result.Num() < count && weighted.Num() > 0 && totalWeight > 0.f )
		{
			const float roll = FMath::FRand() * totalWeight;
			float running = 0.f;
			int32 pickedIndex = weighted.Num() - 1;

			for ( int32 i = 0; i < weighted.Num(); ++i )
			{
				running += weighted[i].Value;
				if ( roll <= running )
				{
					pickedIndex = i;
					break;
				}
			}

			result.Add( weighted[pickedIndex].Key );
			totalWeight -= weighted[pickedIndex].Value;
			weighted.RemoveAtSwap( pickedIndex );
		}

		return result;
	}
}

TArray<UCardDataAsset*> FCardPoolResolver::Resolve(
	const TArray<TObjectPtr<UCardDataAsset>>& pool,
	const TArray<FAppliedCardRecord>& history,
	int32 countToOffer )
{
	TArray<UCardDataAsset*> result;

	if ( pool.Num() == 0 || countToOffer <= 0 )
	{
		return result;
	}

	TSet<UCardDataAsset*> excluded;
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
				excluded.Add( exc );
			}
		}
	}

	TMap<UCardDataAsset*, float> weights;
	weights.Reserve( pool.Num() );

	for ( const TObjectPtr<UCardDataAsset>& card : pool )
	{
		if ( !card )
		{
			continue;
		}
		if ( excluded.Contains( card.Get() ) )
		{
			continue;
		}

		if ( !card->bCanStack )
		{
			bool bAlready = false;
			for ( const FAppliedCardRecord& record : history )
			{
				if ( record.Card == card )
				{
					bAlready = true;
					break;
				}
			}
			if ( bAlready )
			{
				continue;
			}
		}

		weights.Add( card.Get(), card->BaseWeight );
	}

	for ( const FAppliedCardRecord& record : history )
	{
		UCardDataAsset* source = record.Card;
		if ( !source )
		{
			continue;
		}

		const int32 stacks = FMath::Max( 1, record.StackCount );

		auto applyMultiplierMap = [&]( const TMap<TObjectPtr<UCardDataAsset>, float>& map )
		{
			for ( const auto& pair : map )
			{
				UCardDataAsset* target = pair.Key;
				if ( !target )
				{
					continue;
				}
				if ( float* weight = weights.Find( target ) )
				{
					for ( int32 i = 0; i < stacks; ++i )
					{
						*weight *= pair.Value;
					}
				}
			}
		};

		applyMultiplierMap( source->WeightMultipliers_Up );
		applyMultiplierMap( source->WeightMultipliers_Down );
	}

	TArray<TPair<UCardDataAsset*, float>> weighted;
	weighted.Reserve( weights.Num() );
	for ( const auto& pair : weights )
	{
		if ( pair.Value > 0.f )
		{
			weighted.Add( TPair<UCardDataAsset*, float>( pair.Key, pair.Value ) );
		}
	}

	return WeightedSample( weighted, countToOffer );
}
