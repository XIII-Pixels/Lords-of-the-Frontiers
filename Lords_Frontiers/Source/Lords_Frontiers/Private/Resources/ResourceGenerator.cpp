#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"

#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

UResourceGenerator::UResourceGenerator()
{
	ResourceManager_ = nullptr;
}

void UResourceGenerator::Initialize( UResourceManager* manager )
{
	ResourceManager_ = manager;
}

void UResourceGenerator::SetProductionConfig( const FResourceProduction& Config )
{
	BaseProduction_ = Config.ToMap();
}

//hit the bonuses from the cards
TMap<EResourceType, int32> UResourceGenerator::GetTotalProduction() const
{
	TMap<EResourceType, int32> Total = BaseProduction_;

	for ( const auto& Bonus : BonusProduction_ )
	{
		Total.FindOrAdd( Bonus.Key ) += Bonus.Value;
	}
	return Total;
}

void UResourceGenerator::GenerateNow()
{
	ProcessGeneration();
}

void UResourceGenerator::ProcessGeneration()
{
	if ( IsValid( ResourceManager_ ) )
	{
		for ( const auto& Elem : GetTotalProduction() )
		{
			if ( Elem.Value > 0 )
			{
				ResourceManager_->AddResource( Elem.Key, Elem.Value );
			}
		}
	}
}