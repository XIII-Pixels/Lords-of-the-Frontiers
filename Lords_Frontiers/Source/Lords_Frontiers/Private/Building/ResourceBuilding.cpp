#include "Lords_Frontiers/Public/Building/ResourceBuilding.h"

#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"

#include "Kismet/GameplayStatics.h"

static constexpr int32 cDefaultResourceHealth = 200;
static constexpr int32 cDefaultResourceDamage = 0;

AResourceBuilding::AResourceBuilding()
{
	// Init Stats
	Stats_ = FEntityStats( cDefaultResourceHealth, cDefaultResourceDamage, 0.0f );

	// Creating a Generator Component
	ResourceGenerator_ = CreateDefaultSubobject<UResourceGenerator>( TEXT( "ResourceGenerator" ) );
}

void AResourceBuilding::BeginPlay()
{
	Super::BeginPlay();
	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* Eco = PC->FindComponentByClass<UEconomyComponent>() )
		{
			Eco->RegisterBuilding( this );
		}
	}

	// Initialization and start of generation
	UResourceManager* ResourceManager = FindResourceManager_();

	if ( IsValid( ResourceGenerator_ ) && IsValid( ResourceManager ) )
	{
		// Passing the link to the ResourceManager and World Context to the
		// generator
		ResourceGenerator_->Initialize( ResourceManager );
	}
	else
	{
		// Log Warning if resources cannot be managed
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "AResourceBuilding failed to initialize generator or find "
		          "ResourceManager!" )
		);
	}
}

UResourceManager* AResourceBuilding::FindResourceManager_() const
{
	// Assume the ResourceManager is located on the Player Controller
	AController* PlayerController = UGameplayStatics::GetPlayerController( GetWorld(), 0 );

	if ( IsValid( PlayerController ) )
	{
		// need to change if the ResourceManager is located in GameState or
		// PlayerState
		return PlayerController->FindComponentByClass<UResourceManager>();
	}

	return nullptr;
}

void AResourceBuilding::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* Eco = PC->FindComponentByClass<UEconomyComponent>() )
		{
			Eco->UnregisterBuilding( this );
		}
	}
	Super::EndPlay( EndPlayReason );
}