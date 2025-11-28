#include "ResourceManager.h"

UResourceManager::UResourceManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UResourceManager::BeginPlay()
{
	Super::BeginPlay();

	// Initialize resources with zeros, if necessary, or leave the map empty before adding
}

void UResourceManager::AddResource(EResourceType Type, int32 Quantity)
{
	if ((Type == EResourceType::None) || (Quantity <= 0))
	{
		return;
	}

	int32& CurrentAmount = Resources_.FindOrAdd(Type);
	CurrentAmount += Quantity;

	// Here you can add a delegate/event to update the UI
	// OnResourceChanged.Broadcast(Type, CurrentAmount);
}

bool UResourceManager::TrySpendResource(EResourceType Type, int32 Quantity)
{
	if ((Type == EResourceType::None) || (Quantity <= 0))
	{
		return false;
	}

	if (!Resources_.Contains(Type))
	{
		return false;
	}

	int32& CurrentAmount = Resources_.FindOrAdd(Type);

	if (CurrentAmount >= Quantity)
	{
		CurrentAmount -= Quantity;
		// Here you can add a delegate/event to update the UI
		return true;
	}

	return false;
}

int32 UResourceManager::GetResourceAmount(EResourceType Type) const
{
	if (Resources_.Contains(Type))
	{
		return Resources_[Type];
	}

	return 0;
}

bool UResourceManager::HasEnoughResource(EResourceType Type, int32 Quantity) const
{
	return GetResourceAmount(Type) >= Quantity;
}