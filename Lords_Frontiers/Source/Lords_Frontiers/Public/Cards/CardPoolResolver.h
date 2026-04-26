#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"

class UCardDataAsset;

class LORDS_FRONTIERS_API FCardPoolResolver
{
public:
	static TArray<UCardDataAsset*> Resolve(
		const TArray<TObjectPtr<UCardDataAsset>>& pool,
		const TArray<FAppliedCardRecord>& history,
		int32 countToOffer );
};
