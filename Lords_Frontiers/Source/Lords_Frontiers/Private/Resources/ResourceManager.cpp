#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

UResourceManager::UResourceManager()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Map initialization is handled by default constructor or inline
	// initialization in .h
}

void UResourceManager::BeginPlay()
{
	Super::BeginPlay();

	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->RegisterObject( this );
			}
		}
	}
}

void UResourceManager::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->UnregisterObject( this );
			}
		}
	}

	Super::EndPlay( endPlayReason );
}

void UResourceManager::AddResource( EResourceType type, int32 quantity, bool noSound )
{
	if ( type == EResourceType::None || quantity <= 0 )
	{
		return;
	}

	int32& CurrentAmount = Resources_.FindOrAdd( type );
	CurrentAmount = FMath::Min( CurrentAmount + quantity, GetMaxResourceAmount( type ) );
	OnResourceChanged.Broadcast( type, CurrentAmount );

	if ( !noSound )
	{
		switch ( type )
		{
		case EResourceType::Gold:
			OnAudioEvent_.Broadcast( { AudioTags::SFX_RESOURCES_GOLD_AWARDED } );
			break;
		case EResourceType::Food:
			OnAudioEvent_.Broadcast( { AudioTags::SFX_RESOURCES_FOOD_AWARDED } );
			break;
		case EResourceType::Population:
			OnAudioEvent_.Broadcast( { AudioTags::SFX_RESOURCES_POPULATION_AWARDED } );
			break;
		default:
			OnAudioEvent_.Broadcast( { AudioTags::SFX_RESOURCES_DEFAULT_AWARDED } );
		}
	}
}

bool UResourceManager::TrySpendResource( EResourceType type, int32 quantity )
{
	if ( type == EResourceType::None || !Resources_.Contains( type ) )
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
		const int32 value = Pair.Value;
		if ( value > 0 )
		{
			TrySpendResource( Pair.Key, value );
		}
		else if ( value < 0 )
		{
			AddResource( Pair.Key, -value );
		}
	}
}

void UResourceManager::AddResources( const FResourceProduction& amount )
{
	for ( const auto& Pair : amount.ToMap() )
	{
		const int32 value = Pair.Value;
		if ( value > 0 )
		{
			AddResource( Pair.Key, value );
		}
		else if ( value < 0 )
		{
			ForceSpendResource( Pair.Key, -value );
		}
	}
}

int32 UResourceManager::ForceSpendResource( EResourceType type, int32 quantity )
{
	if ( type == EResourceType::None || quantity <= 0 )
	{
		return 0;
	}
	if ( !Resources_.Contains( type ) )
	{
		return 0;
	}

	int32& CurrentAmount = Resources_.FindOrAdd( type );
	int32 AmountToSpend = FMath::Min( CurrentAmount, quantity );

	CurrentAmount -= AmountToSpend;

	if ( AmountToSpend > 0 )
	{
		OnResourceChanged.Broadcast( type, CurrentAmount );
	}
	return AmountToSpend;
}

void UResourceManager::ResetResources()
{
	Resources_.Empty();
	OnResourceChanged.Broadcast( EResourceType::Gold, 0 );
	OnResourceChanged.Broadcast( EResourceType::Food, 0 );
	OnResourceChanged.Broadcast( EResourceType::Population, 0 );
}
