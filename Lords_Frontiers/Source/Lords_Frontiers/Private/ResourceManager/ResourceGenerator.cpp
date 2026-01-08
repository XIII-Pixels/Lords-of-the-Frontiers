#include "Lords_Frontiers/Public/ResourceManager/ResourceGenerator.h"

#include "Lords_Frontiers/Public/ResourceManager/ResourceManager.h"

UResourceGenerator::UResourceGenerator()
{
	ResourceType_ = EResourceType::Gold;
	GenerationQuantity_ = 10;
	ResourceManager_ = nullptr;
}

void UResourceGenerator::Initialize( UResourceManager* manager )
{
	ResourceManager_ = manager;
}

void UResourceGenerator::GenerateNow()
{
	ProcessGeneration();
}

void UResourceGenerator::ProcessGeneration()
{
	if ( IsValid( ResourceManager_ ) )
	{
		ResourceManager_->AddResource( ResourceType_, GenerationQuantity_ );
	}
}