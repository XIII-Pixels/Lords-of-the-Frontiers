#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"

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
	// Save original production for card reset on game restart. Must happen before
	// Super::BeginPlay(), which already applies the active cards (OnBuildingPlaced).
	OriginalProductionConfig_ = ProductionConfig_;
	ProductionClampDebt_ = FResourceProduction();

	Super::BeginPlay();

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
	ProductionConfig_.ModifyByTypeReversible( type, delta, ProductionClampDebt_ );
	SyncGeneratorConfig();
}

void AResourceBuilding::ModifyProductionAll( int32 delta )
{
	for ( EResourceType resType : CardTypeHelpers::GetAllResourceTypes() )
	{
		// Only resources this building actually produces: a farm must not start producing gold,
		// population and progress because of an "all resources" bonus.
		if ( OriginalProductionConfig_.GetByType( resType ) <= 0 )
		{
			continue;
		}
		ProductionConfig_.ModifyByTypeReversible( resType, delta, ProductionClampDebt_ );
	}
	SyncGeneratorConfig();
}

void AResourceBuilding::ResetProductionToDefaults()
{
	ProductionConfig_ = OriginalProductionConfig_;
	ProductionClampDebt_ = FResourceProduction();
	SyncGeneratorConfig();
}

void AResourceBuilding::SyncGeneratorConfig()
{
	if ( IsValid( ResourceGenerator_ ) )
	{
		ResourceGenerator_->SetProductionConfig( ProductionConfig_ );
	}
}
