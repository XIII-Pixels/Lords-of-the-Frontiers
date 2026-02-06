#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"

#include "Building/ResourceBuilding.h"
#include "Cards/CardSubsystem.h"
#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"
#include "Lords_Frontiers/Public/Grid/GridManager.h"
#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"

UEconomyComponent::UEconomyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GridManager_ = nullptr;
	ResourceManager_ = nullptr;
}

void UEconomyComponent::BeginPlay()
{
	Super::BeginPlay();
	FindSystems();
}

void UEconomyComponent::FindSystems()
{
	GridManager_ = Cast<AGridManager>( UGameplayStatics::GetActorOfClass( GetWorld(), AGridManager::StaticClass() ) );

	AActor* Owner = GetOwner();
	if ( Owner )
	{
		ResourceManager_ = Owner->FindComponentByClass<UResourceManager>();
	}

	if ( !GridManager_ )
	{
		UE_LOG( LogTemp, Warning, TEXT( "EconomyComponent: GridManager not found!" ) );
	}
	if ( !ResourceManager_ )
	{
		UE_LOG( LogTemp, Warning, TEXT( "EconomyComponent: ResourceManager not found on owner!" ) );
	}
}

void UEconomyComponent::RegisterBuilding( ABuilding* building )
{
	if ( building )
	{
		RegisteredBuildings_.AddUnique( building );
	}
	MarkEconomyDirty();
}

void UEconomyComponent::UnregisterBuilding( ABuilding* building )
{
	RegisteredBuildings_.Remove( building );

	MarkEconomyDirty();
}

void UEconomyComponent::PerformInitialScan()
{
	if ( bInitialScanDone )
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ABuilding::StaticClass(), FoundActors );

	for ( AActor* Actor : FoundActors )
	{
		if ( ABuilding* B = Cast<ABuilding>( Actor ) )
		{
			RegisterBuilding( B );
		}
	}

	bInitialScanDone = true;
	UE_LOG(
	    LogTemp, Warning, TEXT( "Economy: Initial Scan completed. Found %d buildings." ), RegisteredBuildings_.Num()
	);

	MarkEconomyDirty();
}

void UEconomyComponent::CollectGlobalResources()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Orange, TEXT( "Economy: Collecting resources..." ) );
	}

	if ( !ResourceManager_ )
	{
		FindSystems();
		if ( !ResourceManager_ )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Red, TEXT( "Error: ResourceManager is NULL" ) );
			}
			return;
		}
	}

	PerformInitialScan();

	if ( RegisteredBuildings_.Num() == 0 )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 5.f, FColor::Red, TEXT( "Error: No buildings registered even after discovery!" )
			);
		}
		return;
	}

	TMap<EResourceType, int32> collectedTotals;

	for ( auto It = RegisteredBuildings_.CreateIterator(); It; ++It )
	{
		if ( ABuilding* B = It->Get() )
		{
			if ( AResourceBuilding* resBuilding = Cast<AResourceBuilding>( B ) )
			{
				if ( UResourceGenerator* gen = resBuilding->GetResourceGenerator() )
				{
					TMap<EResourceType, int32> productionMap = gen->GetTotalProduction();

					for ( const auto& pair : productionMap )
					{
						if ( pair.Key != EResourceType::None && pair.Value > 0 )
						{
							collectedTotals.FindOrAdd( pair.Key ) += pair.Value;
						}
					}
				}
			}
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this );
	if ( cardSubsystem )
	{
		const FEconomyBonuses& bonuses = cardSubsystem->GetEconomyBonuses();

		auto addBonus = [&]( EResourceType type, int32 bonus )
		{
			if ( bonus > 0 )
			{
				collectedTotals.FindOrAdd( type ) += bonus;
			}
		};

		addBonus( EResourceType::Gold, bonuses.GoldProductionBonus );
		addBonus( EResourceType::Food, bonuses.FoodProductionBonus );
		addBonus( EResourceType::Population, bonuses.PopulationProductionBonus );
		addBonus( EResourceType::Progress, bonuses.ProgressProductionBonus );
	}

	for ( const auto& Elem : collectedTotals )
	{
		ResourceManager_->AddResource( Elem.Key, Elem.Value );
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 5.f, FColor::Green, FString::Printf( TEXT( "Added %d of type %d" ), Elem.Value, (uint8) Elem.Key )
			);
		}
	}
}

void UEconomyComponent::ApplyMaintenanceCosts()
{
	if ( !ResourceManager_ )
	{
		return;
	}

	PerformInitialScan();

	TMap<EResourceType, int32> TotalCosts;

	for ( auto It = RegisteredBuildings_.CreateIterator(); It; ++It )
	{
		if ( ABuilding* B = It->Get() )
		{
			TMap<EResourceType, int32> Costs = B->GetMaintenanceCost().ToMap();
			for ( const auto& Pair : Costs )
			{
				if ( Pair.Value > 0 )
				{
					TotalCosts.FindOrAdd( Pair.Key ) += Pair.Value;
				}
			}
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this );
	if ( cardSubsystem )
	{
		const FEconomyBonuses& bonuses = cardSubsystem->GetEconomyBonuses();

		auto applyReduction = [&]( EResourceType type, int32 reduction )
		{
			if ( reduction > 0 && TotalCosts.Contains( type ) )
			{
				TotalCosts[type] = FMath::Max( 0, TotalCosts[type] - reduction );
			}
		};

		applyReduction( EResourceType::Gold, bonuses.GoldMaintenanceReduction );
		applyReduction( EResourceType::Food, bonuses.FoodMaintenanceReduction );
		applyReduction( EResourceType::Population, bonuses.PopulationMaintenanceReduction );
		applyReduction( EResourceType::Progress, bonuses.ProgressMaintenanceReduction );
	}

	for ( const auto& Pair : TotalCosts )
	{
		if ( Pair.Value > 0 )
		{
			ResourceManager_->TrySpendResource( Pair.Key, Pair.Value );
		}
	}
}

void UEconomyComponent::RestoreAllBuildings()
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ABuilding::StaticClass(), foundActors );

	for ( AActor* actor : foundActors )
	{
		if ( ABuilding* b = Cast<ABuilding>( actor ) )
		{
			b->RestoreFromRuins();
		}
	}
}

FResourceProduction UEconomyComponent::GetTotalProduction()
{
	RecalculateIfDirty();
	return CachedTotalProduction_;
}

FResourceProduction UEconomyComponent::GetTotalMaintenance()
{
	RecalculateIfDirty();
	return CachedTotalMaintenance_;
}

FResourceProduction UEconomyComponent::GetNetIncome()
{
	RecalculateIfDirty();
	return CachedNetIncome_;
}

int32 UEconomyComponent::GetNetIncomeForType( EResourceType Type )
{
	RecalculateIfDirty();
	return CachedNetIncome_.GetByType( Type );
}

void UEconomyComponent::MarkEconomyDirty()
{
	bEconomyDirty_ = true;
	RecalculateIfDirty();
}

void UEconomyComponent::RecalculateIfDirty()
{
	if ( bEconomyDirty_ )
	{
		RecalculateEconomyBalance();
		bEconomyDirty_ = false;
	}
}

void UEconomyComponent::RecalculateEconomyBalance()
{
	const FResourceProduction PreviousNet = CachedNetIncome_;

	CachedTotalProduction_ = FResourceProduction();
	CachedTotalMaintenance_ = FResourceProduction();
	CachedNetIncome_ = FResourceProduction();

	for ( int32 i = RegisteredBuildings_.Num() - 1; i >= 0; --i )
	{
		if ( !RegisteredBuildings_[i].IsValid() )
		{
			RegisteredBuildings_.RemoveAtSwap( i );
		}
	}

	for ( const TWeakObjectPtr<ABuilding>& WeakBuilding : RegisteredBuildings_ )
	{
		ABuilding* Building = WeakBuilding.Get();
		if ( !Building || Building->IsDestroyed() )
		{
			continue;
		}

		const FResourceProduction& Maint = Building->GetMaintenanceCost();
		CachedTotalMaintenance_.Gold += Maint.Gold;
		CachedTotalMaintenance_.Food += Maint.Food;
		CachedTotalMaintenance_.Population += Maint.Population;
		CachedTotalMaintenance_.Progress += Maint.Progress;

		if ( AResourceBuilding* ResBuild = Cast<AResourceBuilding>( Building ) )
		{
			const FResourceProduction& Prod = ResBuild->GetProductionConfig();
			CachedTotalProduction_.Gold += Prod.Gold;
			CachedTotalProduction_.Food += Prod.Food;
			CachedTotalProduction_.Population += Prod.Population;
			CachedTotalProduction_.Progress += Prod.Progress;
		}
	}

	CachedNetIncome_.Gold = CachedTotalProduction_.Gold - CachedTotalMaintenance_.Gold;
	CachedNetIncome_.Food = CachedTotalProduction_.Food - CachedTotalMaintenance_.Food;
	CachedNetIncome_.Population = CachedTotalProduction_.Population - CachedTotalMaintenance_.Population;
	CachedNetIncome_.Progress = CachedTotalProduction_.Progress - CachedTotalMaintenance_.Progress;

	const bool bChanged = PreviousNet.Gold != CachedNetIncome_.Gold || PreviousNet.Food != CachedNetIncome_.Food ||
	                      PreviousNet.Population != CachedNetIncome_.Population ||
	                      PreviousNet.Progress != CachedNetIncome_.Progress;

	if ( bChanged )
	{
		OnEconomyBalanceChanged.Broadcast( CachedNetIncome_ );
	}
}