#pragma once

#include "Cards/CardCondition.h"

#include "CoreMinimal.h"

#include "CardCondition_Always.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Condition: Always" ) )
class LORDS_FRONTIERS_API UCardCondition_Always : public UCardCondition
{
	GENERATED_BODY()

public:
	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const override
	{
		return true;
	}

	virtual FText GetDisplayText_Implementation() const override
	{
		return FText::FromString( TEXT( "Always" ) );
	}
};
