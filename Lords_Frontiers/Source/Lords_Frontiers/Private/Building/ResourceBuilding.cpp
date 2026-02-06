#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"

#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"

static constexpr int32 cDefaultResourceHealth = 200;
static constexpr int32 cDefaultResourceDamage = 0;

AResourceBuilding::AResourceBuilding()
{
	Stats_ = FEntityStats( cDefaultResourceHealth, cDefaultResourceDamage, 0.0f );
	ResourceGenerator_ = CreateDefaultSubobject<UResourceGenerator>( TEXT( "ResourceGenerator" ) );
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
