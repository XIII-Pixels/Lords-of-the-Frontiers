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

void UResourceGenerator::SetProductionConfig( const TArray<FGameResource>& Config )
{
	ProductionResources_ = Config;
}

void UResourceGenerator::GenerateNow()
{
	ProcessGeneration();
}

void UResourceGenerator::ProcessGeneration()
{
	if ( IsValid( ResourceManager_ ) )
	{
		for ( const FGameResource& Res : ProductionResources_ )
		{
			ResourceManager_->AddResource( Res.Type, Res.Quantity );
		}
	}
}