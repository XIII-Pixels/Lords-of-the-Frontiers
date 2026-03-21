#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"

#include "Building/Animation/ResourceAnimConfig.h"
#include "Building/Animation/ResourceCollectionAnimComponent.h"
#include "Building/MainBase.h"
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

	RecalculateAndBroadcastNetIncome();
}

void UEconomyComponent::UnregisterBuilding( ABuilding* building )
{
	RegisteredBuildings_.Remove( building );

	RecalculateAndBroadcastNetIncome();
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

	RecalculateAndBroadcastNetIncome();
}

FResourceProduction UEconomyComponent::CalculateTotalIncome() const
{
	FResourceProduction totalIncome;

	for ( const TWeakObjectPtr<ABuilding>& weakBuilding : RegisteredBuildings_ )
	{
		ABuilding* building = weakBuilding.Get();
		if ( !IsValid( building ) || building->IsRuined() )
		{
			continue;
		}

		AResourceBuilding* resBuilding = Cast<AResourceBuilding>( building );
		if ( !resBuilding )
		{
			continue;
		}

		UResourceGenerator* gen = resBuilding->GetResourceGenerator();
		if ( !IsValid( gen ) )
		{
			continue;
		}

		TMap<EResourceType, int32> productionMap = gen->GetTotalProduction();
		for ( const auto& pair : productionMap )
		{
			if ( pair.Key != EResourceType::None && pair.Value > 0 )
			{
				totalIncome.ModifyByType( pair.Key, pair.Value );
			}
		}
	}

	UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this );
	if ( cardSubsystem )
	{
		const FEconomyBonuses& bonuses = cardSubsystem->GetEconomyBonuses();
		if ( bonuses.GoldProductionBonus > 0 )
		{
			totalIncome.ModifyByType( EResourceType::Gold, bonuses.GoldProductionBonus );
		}
		if ( bonuses.FoodProductionBonus > 0 )
		{
			totalIncome.ModifyByType( EResourceType::Food, bonuses.FoodProductionBonus );
		}
		if ( bonuses.PopulationProductionBonus > 0 )
		{
			totalIncome.ModifyByType( EResourceType::Population, bonuses.PopulationProductionBonus );
		}
		if ( bonuses.ProgressProductionBonus > 0 )
		{
			totalIncome.ModifyByType( EResourceType::Progress, bonuses.ProgressProductionBonus );
		}
	}

	return totalIncome;
}

FResourceProduction UEconomyComponent::CalculateTotalMaintenance() const
{
	FResourceProduction totalMaintenance;

	for ( const TWeakObjectPtr<ABuilding>& weakBuilding : RegisteredBuildings_ )
	{
		ABuilding* building = weakBuilding.Get();
		if ( !IsValid( building ) )
		{
			continue;
		}

		FResourceProduction costs = building->GetMaintenanceCost();
		if ( costs.Gold > 0 )
		{
			totalMaintenance.ModifyByType( EResourceType::Gold, costs.Gold );
		}
		if ( costs.Food > 0 )
		{
			totalMaintenance.ModifyByType( EResourceType::Food, costs.Food );
		}
		if ( costs.Population > 0 )
		{
			totalMaintenance.ModifyByType( EResourceType::Population, costs.Population );
		}
		if ( costs.Progress > 0 )
		{
			totalMaintenance.ModifyByType( EResourceType::Progress, costs.Progress );
		}
	}

	UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this );
	if ( cardSubsystem )
	{
		const FEconomyBonuses& bonuses = cardSubsystem->GetEconomyBonuses();

		auto applyReduction = [&]( EResourceType type, int32 reduction )
		{
			if ( reduction > 0 )
			{
				int32 current = totalMaintenance.GetByType( type );
				totalMaintenance.SetByType( type, FMath::Max( 0, current - reduction ) );
			}
		};

		applyReduction( EResourceType::Gold, bonuses.GoldMaintenanceReduction );
		applyReduction( EResourceType::Food, bonuses.FoodMaintenanceReduction );
		applyReduction( EResourceType::Population, bonuses.PopulationMaintenanceReduction );
		applyReduction( EResourceType::Progress, bonuses.ProgressMaintenanceReduction );
	}

	return totalMaintenance;
}

FResourceProduction UEconomyComponent::CalculateNetIncome() const
{
	return CalculateTotalIncome() - CalculateTotalMaintenance();
}

void UEconomyComponent::RecalculateAndBroadcastNetIncome()
{
	FResourceProduction netIncome = CalculateNetIncome();
	if ( netIncome != CachedNetIncome_ )
	{
		CachedNetIncome_ = netIncome;
		OnNetIncomeChanged.Broadcast( netIncome );
	}
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

	FResourceProduction income = CalculateTotalIncome();
	TMap<EResourceType, int32> collectedTotals = income.ToMap();

	for ( const auto& elem : collectedTotals )
	{
		if ( elem.Value > 0 )
		{
			ResourceManager_->AddResource( elem.Key, elem.Value );
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage(
				    -1, 5.f, FColor::Green,
				    FString::Printf( TEXT( "Added %d of type %d" ), elem.Value, (uint8) elem.Key )
				);
			};
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

	FResourceProduction maintenance = CalculateTotalMaintenance();
	TMap<EResourceType, int32> totalCosts = maintenance.ToMap();

	for ( const auto& pair : totalCosts )
	{

		if ( pair.Value > 0 )
		{
			ResourceManager_->ForceSpendResource( pair.Key, pair.Value );
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

	RecalculateAndBroadcastNetIncome();
}

static FBuildingCollectionAnimData BuildAnimData( ABuilding* building )
{
	FBuildingCollectionAnimData data;

	if ( building->IsRuined() )
	{
		data.bIsRuined = true;
		const FResourceProduction& maintenance = building->GetMaintenanceCost();

		for ( EResourceType type : CardTypeHelpers::GetAllResourceTypes() )
		{
			const int32 cost = maintenance.GetByType( type );
			if ( cost > 0 )
			{
				data.BaseIncome.Add( { type, -cost } );
			}
		}
		return data;
	}

	AResourceBuilding* resBuild = Cast<AResourceBuilding>( building );
	if ( resBuild )
	{
		data = resBuild->GetCollectionAnimData();
	}

	return data;
}

void UEconomyComponent::TriggerCollectionAnimations()
{
	PerformInitialScan();

	FVector baseLocation = FVector::ZeroVector;
	const float waveRate = ( ResourceAnimConfig_ ? ResourceAnimConfig_->WaveDelayPerUnit : 0.0f );

	if ( waveRate > 0.0f )
	{
		if ( const AMainBase* mainBase =
		         Cast<AMainBase>( UGameplayStatics::GetActorOfClass( GetWorld(), AMainBase::StaticClass() ) ) )
		{
			baseLocation = mainBase->GetActorLocation();
		}
	}

	TArray<FResourcePopupBatchEntry> batch;

	for ( const TWeakObjectPtr<ABuilding>& weakBuilding : RegisteredBuildings_ )
	{
		ABuilding* building = weakBuilding.Get();
		if ( !IsValid( building ) )
		{
			continue;
		}

		FBuildingCollectionAnimData data = BuildAnimData( building );
		if ( data.IsEmpty() )
		{
			continue;
		}

		const float waveDelay =
		    ( waveRate > 0.0f ) ? FVector::Dist2D( baseLocation, building->GetActorLocation() ) * waveRate : 0.0f;

		UResourceCollectionAnimComponent* animComp = building->FindComponentByClass<UResourceCollectionAnimComponent>();
		if ( animComp )
		{
			animComp->StartAnimation( ResourceAnimConfig_, data.bIsRuined, waveDelay );
		}

		FResourcePopupBatchEntry entry;
		entry.BuildingWorldLocation = building->GetActorLocation();
		entry.AnimData = data;
		entry.WaveDelay = waveDelay;
		batch.Add( MoveTemp( entry ) );
	}

	if ( batch.Num() > 0 )
	{
		OnCollectionAnimRequested.Broadcast( batch );
	}
}

void UEconomyComponent::SubscribeToCardEvents()
{
	UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this );
	if ( !cardSubsystem )
	{
		return;
	}

	cardSubsystem->OnCardsApplied.AddDynamic( this, &UEconomyComponent::HandleCardsApplied );
	cardSubsystem->OnEconomyBonusesChanged.AddDynamic( this, &UEconomyComponent::HandleEconomyBonusesChanged );
}

void UEconomyComponent::HandleCardsApplied( const TArray<UCardDataAsset*>& appliedCards )
{
	RecalculateAndBroadcastNetIncome();
}

void UEconomyComponent::HandleEconomyBonusesChanged( const FEconomyBonuses& newBonuses )
{
	RecalculateAndBroadcastNetIncome();
}

void UEconomyComponent::ResetEconomy()
{
	RegisteredBuildings_.Empty();
	bInitialScanDone = false;
	CachedNetIncome_ = FResourceProduction();

	FindSystems();
}