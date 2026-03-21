#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"

#include "Building/Bonus/BuildingBonusComponent.h"
#include "Cards/CardTypes.h"
#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"

static constexpr int32 cDefaultResourceHealth = 200;
static constexpr int32 cDefaultResourceDamage = 0;

AResourceBuilding::AResourceBuilding()
{
	Stats_ = FEntityStats( cDefaultResourceHealth, cDefaultResourceDamage, 0.0f );
	ResourceGenerator_ = CreateDefaultSubobject<UResourceGenerator>( TEXT( "ResourceGenerator" ) );
	Stats_.SetTeam( ETeam::Cat );
}

void AResourceBuilding::BeginPlay()
{
	Super::BeginPlay();

	// Save original production for card reset on game restart
	OriginalProductionConfig_ = ProductionConfig_;

	if ( IsValid( ResourceGenerator_ ) )
	{
		ResourceGenerator_->Initialize( FindResourceManager() );
		ResourceGenerator_->SetProductionConfig( ProductionConfig_ );
	}

	if ( EconomyComponent_ )
	{
		EconomyComponent_->RecalculateAndBroadcastNetIncome();
	}
}

UResourceManager* AResourceBuilding::FindResourceManager() const
{
	AController* PlayerController = UGameplayStatics::GetPlayerController( GetWorld(), 0 );

	if ( IsValid( PlayerController ) )
	{
		return PlayerController->FindComponentByClass<UResourceManager>();
	}

	return nullptr;
}

void AResourceBuilding::ModifyProduction( EResourceType type, int32 delta )
{
	ProductionConfig_.ModifyByType( type, delta );
	SyncGeneratorConfig();
}

void AResourceBuilding::ModifyProductionAll( int32 delta )
{
	for ( EResourceType resType : CardTypeHelpers::GetAllResourceTypes() )
	{
		ProductionConfig_.ModifyByType( resType, delta );
	}
	SyncGeneratorConfig();
}

void AResourceBuilding::ResetProductionToDefaults()
{
	ProductionConfig_ = OriginalProductionConfig_;
	SyncGeneratorConfig();
}

void AResourceBuilding::SyncGeneratorConfig()
{
	if ( IsValid( ResourceGenerator_ ) )
	{
		ResourceGenerator_->SetProductionConfig( ProductionConfig_ );
	}
}

FBuildingCollectionAnimData AResourceBuilding::GetCollectionAnimData() const
{
	FBuildingCollectionAnimData data;

	const FResourceProduction& maintenance = GetMaintenanceCost();

	if ( IsRuined() )
	{
		data.bIsRuined = true;
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

	FResourceProduction bonusProduction;
	const UBuildingBonusComponent* bonusComp = FindComponentByClass<UBuildingBonusComponent>();
	if ( bonusComp )
	{
		bonusProduction = bonusComp->GetBonusResourceProduction();
	}

	TMap<EResourceType, int32> totalProduction;
	if ( IsValid( ResourceGenerator_ ) )
	{
		totalProduction = ResourceGenerator_->GetTotalProduction();
	}

	for ( EResourceType type : CardTypeHelpers::GetAllResourceTypes() )
	{
		const int32 total = totalProduction.Contains( type ) ? totalProduction[type] : 0;
		const int32 bonus = bonusProduction.GetByType( type );
		const int32 baseProduction = total - bonus;
		const int32 maintenanceCost = maintenance.GetByType( type );
		const int32 netBase = baseProduction - maintenanceCost;

		if ( netBase != 0 )
		{
			data.BaseIncome.Add( { type, netBase } );
		}
		if ( bonus != 0 )
		{
			data.BonusIncome.Add( { type, bonus } );
		}
	}

	return data;
}