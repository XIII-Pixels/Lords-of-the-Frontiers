#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

UResourceManager::UResourceManager()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Map initialization is handled by default constructor or inline
	// initialization in .h
}

void UResourceManager::BeginPlay()
{
	Super::BeginPlay();
}

void UResourceManager::AddResource( EResourceType type, int32 quantity )
{
	if ( type == EResourceType::None || quantity <= 0 )
	{
		return;
	}

	int32& CurrentAmount = Resources_.FindOrAdd( type );
	const int32 MaxAmount = GetMaxResourceAmount( type );

	CurrentAmount = FMath::Min( CurrentAmount + quantity, MaxAmount );
	OnResourceChanged.Broadcast( type, CurrentAmount );
}

bool UResourceManager::TrySpendResource( EResourceType type, int32 quantity )
{
	if ( ( type == EResourceType::None ) )
	{
		return false;
	}

	if ( !Resources_.Contains( type ) )
	{
		return false;
	}

	int32& CurrentAmount = Resources_.FindOrAdd( type );

	if ( CurrentAmount >= quantity )
	{
		CurrentAmount -= quantity;
		OnResourceChanged.Broadcast( type, CurrentAmount );
		return true;
	}

	return false;
}

int32 UResourceManager::GetResourceAmount( EResourceType type ) const
{
	if ( Resources_.Contains( type ) )
	{
		return Resources_[type];
	}

	return 0;
}

bool UResourceManager::HasEnoughResource( EResourceType type, int32 quantity ) const
{
	return GetResourceAmount( type ) >= quantity;
}

int32 UResourceManager::GetMaxResourceAmount( EResourceType type ) const
{
	return MaxResources_.Contains( type ) ? MaxResources_[type] : cDefaultMaxResource;
}

bool UResourceManager::CanAfford( const FResourceProduction& cost ) const
{
	for ( const auto& Pair : cost.ToMap() )
	{
		if ( Pair.Value > 0 && GetResourceAmount( Pair.Key ) < Pair.Value )
		{
			return false;
		}
	}
	return true;
}

void UResourceManager::SpendResources( const FResourceProduction& cost )
{
	for ( const auto& Pair : cost.ToMap() )
	{
		TrySpendResource( Pair.Key, Pair.Value );
	}
}