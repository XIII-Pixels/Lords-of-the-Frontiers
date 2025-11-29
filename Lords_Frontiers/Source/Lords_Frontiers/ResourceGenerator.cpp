#include "ResourceGenerator.h"
#include "ResourceManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UResourceGenerator::UResourceGenerator()
{
	ResourceType_ = EResourceType::Gold;
	GenerationQuantity_ = 10;
	GenerationInterval_ = cDefaultInterval;
	ResourceManager_ = nullptr;
}

void UResourceGenerator::Initialize(UResourceManager* manager, UObject* worldContext)
{
	ResourceManager_ = manager;
	WorldContext_ = worldContext;
}

void UResourceGenerator::StartGeneration()
{
	UWorld* World = GetWorldContext();

	// Logic check: if interval is valid and world exists
	if ( IsValid( World ) && ( GenerationInterval_ > 0.0f ) )
	{
		World->GetTimerManager().SetTimer(
			GenerationTimerHandle_,
			this,
			&UResourceGenerator::ProcessGeneration,
			GenerationInterval_,
			true
		);
	}
}

void UResourceGenerator::StopGeneration()
{
	UWorld* World = GetWorldContext();
	if ( IsValid( World ) )
	{
		World->GetTimerManager().ClearTimer( GenerationTimerHandle_ );
	}
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

UWorld* UResourceGenerator::GetWorldContext() const
{
	if ( WorldContext_.IsValid() )
	{
		return WorldContext_->GetWorld();
	}
	return nullptr;
}