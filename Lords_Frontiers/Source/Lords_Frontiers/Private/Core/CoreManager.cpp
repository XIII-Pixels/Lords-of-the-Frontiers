#include "Core/CoreManager.h"

#include "Building/Construction/BuildManager.h"
#include "Core/GameLoopManager.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "Grid/GridManager.h"
#include "Grid/GridVisualizer.h"
#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"
#include "Waves/WaveManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC( LogCoreManager, Log, All );

UCoreManager* UCoreManager::Get( const UObject* worldContextObject )
{
	if ( !worldContextObject )
	{
		return nullptr;
	}

	if ( !GEngine )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "Get: GEngine is null" ) );
		return nullptr;
	}

	UWorld* world = GEngine->GetWorldFromContextObject( worldContextObject, EGetWorldErrorMode::LogAndReturnNull );
	if ( !world )
	{
		return nullptr;
	}

	UGameInstance* gi = world->GetGameInstance();
	if ( !gi )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "Get: GameInstance is null" ) );
		return nullptr;
	}

	return gi->GetSubsystem<UCoreManager>();
}

void UCoreManager::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );
	UE_LOG( LogCoreManager, Log, TEXT( "CoreManager initialized" ) );
}

void UCoreManager::Deinitialize()
{
	if ( GameLoopManager_ )
	{
		GameLoopManager_->Cleanup();
	}

	ClearAllReferences();
	OnSystemsReady.Clear();
	bIsInitialized_ = false;

	UE_LOG( LogCoreManager, Log, TEXT( "CoreManager deinitialized" ) );
	Super::Deinitialize();
}

void UCoreManager::InitializeSystems()
{
	if ( bIsInitialized_ )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "InitializeSystems called but already initialized" ) );
		return;
	}

	UE_LOG( LogCoreManager, Log, TEXT( "=== CoreManager: Initializing Systems ===" ) );

	FindWorldActors();
	FindPlayerControllerComponents();
	CreateInternalManagers();
	SetupManagerConnections();

	bIsInitialized_ = true;

#if !UE_BUILD_SHIPPING
	LogSystemsStatus();
#endif

	if ( AreCriticalSystemsReady() )
	{
		OnSystemsReady.Broadcast();
		UE_LOG( LogCoreManager, Log, TEXT( "=== CoreManager: Critical Systems Ready ===" ) );
	}
	else
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "=== CoreManager: Critical Systems Missing! ===" ) );
	}
}

void UCoreManager::RefreshSystemReferences()
{
	UE_LOG( LogCoreManager, Log, TEXT( "CoreManager: Refreshing system references..." ) );

	CachedPlayerController_.Reset();
	CachedPlayerIndex_ = -1;

	bool bWasCriticalReady = AreCriticalSystemsReady();

	FindWorldActors();
	FindPlayerControllerComponents();
	UpdateGameLoopDependencies();

#if !UE_BUILD_SHIPPING
	LogSystemsStatus();
#endif

	if ( !bWasCriticalReady && AreCriticalSystemsReady() )
	{
		OnSystemsReady.Broadcast();
		UE_LOG( LogCoreManager, Log, TEXT( "CoreManager: Systems became ready after refresh" ) );
	}
}

void UCoreManager::ResetSystems()
{
	UE_LOG( LogCoreManager, Log, TEXT( "CoreManager: Resetting all systems..." ) );

	if ( GameLoopManager_ )
	{
		GameLoopManager_->Cleanup();
	}

	ClearAllReferences();
	bIsInitialized_ = false;
}

void UCoreManager::ClearAllReferences()
{
	WaveManager_.Reset();
	BuildManager_.Reset();
	GridVisualizer_.Reset();
	GridManager_.Reset();
	ResourceManager_.Reset();
	EconomyComponent_.Reset();
	SelectionManager_.Reset();
	CachedPlayerController_.Reset();
	CachedPlayerIndex_ = -1;

	GameLoopManager_ = nullptr;
}

void UCoreManager::UpdateGameLoopDependencies()
{
	if ( !GameLoopManager_ )
	{
		return;
	}

	GameLoopManager_->UpdateDependencies( WaveManager_.Get(), ResourceManager_.Get(), EconomyComponent_.Get() );
}

bool UCoreManager::AreAllSystemsReady() const
{
	return WaveManager_.IsValid() && BuildManager_.IsValid() && GridVisualizer_.IsValid() && GridManager_.IsValid() &&
	       ResourceManager_.IsValid() && EconomyComponent_.IsValid() && GameLoopManager_ != nullptr;
}

bool UCoreManager::AreCriticalSystemsReady() const
{
	return ResourceManager_.IsValid() && GameLoopManager_ != nullptr;
}

AWaveManager* UCoreManager::GetWaveManager() const
{
	return WaveManager_.Get();
}

ABuildManager* UCoreManager::GetBuildManager() const
{
	return BuildManager_.Get();
}

AGridVisualizer* UCoreManager::GetGridVisualizer() const
{
	return GridVisualizer_.Get();
}

AGridManager* UCoreManager::GetGridManager() const
{
	return GridManager_.Get();
}

UResourceManager* UCoreManager::GetResourceManager() const
{
	return ResourceManager_.Get();
}

UEconomyComponent* UCoreManager::GetEconomyComponent() const
{
	return EconomyComponent_.Get();
}

USelectionManagerComponent* UCoreManager::GetSelectionManager() const
{
	return SelectionManager_.Get();
}

UGameLoopManager* UCoreManager::GetGameLoop() const
{
	return GameLoopManager_;
}

UWorld* UCoreManager::GetWorldSafe() const
{
	UGameInstance* gi = GetGameInstance();
	if ( !gi )
	{
		return nullptr;
	}
	return gi->GetWorld();
}

APlayerController* UCoreManager::GetPlayerController( int32 playerIndex ) const
{
	UWorld* world = GetWorldSafe();
	if ( !world )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "GetPlayerController: No valid World" ) );
		return nullptr;
	}

	if ( CachedPlayerController_.IsValid() && CachedPlayerIndex_ == playerIndex &&
	     CachedPlayerController_->GetWorld() == world )
	{
		return CachedPlayerController_.Get();
	}

	APlayerController* pc = UGameplayStatics::GetPlayerController( world, playerIndex );
	if ( pc )
	{
		CachedPlayerController_ = pc;
		CachedPlayerIndex_ = playerIndex;
	}
	else
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "GetPlayerController: No PlayerController at index %d" ), playerIndex );
	}

	return pc;
}

template <typename T>
void UCoreManager::RegisterManagerInternal( TWeakObjectPtr<T>& managerRef, T* inManager, const TCHAR* managerName )
{
	if ( !inManager )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "Register%s: null pointer" ), managerName );
		return;
	}

	if ( managerRef.IsValid() && managerRef.Get() != inManager )
	{
		UE_LOG(
		    LogCoreManager, Warning, TEXT( "Register%s: replacing %s with %s" ), managerName, *managerRef->GetName(),
		    *inManager->GetName()
		);
	}

	managerRef = inManager;
	UE_LOG( LogCoreManager, Log, TEXT( "%s registered: %s" ), managerName, *inManager->GetName() );
}

template <typename T>
void UCoreManager::UnregisterManagerInternal( TWeakObjectPtr<T>& managerRef, T* inManager, const TCHAR* managerName )
{
	if ( managerRef.Get() == inManager )
	{
		UE_LOG(
		    LogCoreManager, Log, TEXT( "%s unregistered: %s" ), managerName,
		    inManager ? *inManager->GetName() : TEXT( "null" )
		);
		managerRef.Reset();
	}
}

void UCoreManager::RegisterWaveManager( AWaveManager* inWaveManager )
{
	RegisterManagerInternal( WaveManager_, inWaveManager, TEXT( "WaveManager" ) );
}

void UCoreManager::UnregisterWaveManager( AWaveManager* inWaveManager )
{
	UnregisterManagerInternal( WaveManager_, inWaveManager, TEXT( "WaveManager" ) );
}

void UCoreManager::RegisterBuildManager( ABuildManager* inBuildManager )
{
	RegisterManagerInternal( BuildManager_, inBuildManager, TEXT( "BuildManager" ) );
}

void UCoreManager::UnregisterBuildManager( ABuildManager* inBuildManager )
{
	UnregisterManagerInternal( BuildManager_, inBuildManager, TEXT( "BuildManager" ) );
}

void UCoreManager::RegisterGridVisualizer( AGridVisualizer* inGridVisualizer )
{
	RegisterManagerInternal( GridVisualizer_, inGridVisualizer, TEXT( "GridVisualizer" ) );
}

void UCoreManager::UnregisterGridVisualizer( AGridVisualizer* inGridVisualizer )
{
	UnregisterManagerInternal( GridVisualizer_, inGridVisualizer, TEXT( "GridVisualizer" ) );
}

void UCoreManager::RegisterGridManager( AGridManager* InGridManager )
{
	RegisterManagerInternal( GridManager_, InGridManager, TEXT( "GridManager" ) );
}

void UCoreManager::UnregisterGridManager( AGridManager* InGridManager )
{
	UnregisterManagerInternal( GridManager_, InGridManager, TEXT( "GridManager" ) );
}

void UCoreManager::FindWorldActors()
{
	UWorld* world = GetWorldSafe();
	if ( !world )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "FindWorldActors: No valid World" ) );
		return;
	}

	if ( !WaveManager_.IsValid() )
	{
		AActor* found = UGameplayStatics::GetActorOfClass( world, AWaveManager::StaticClass() );
		WaveManager_ = Cast<AWaveManager>( found );
	}

	if ( !BuildManager_.IsValid() )
	{
		AActor* found = UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() );
		BuildManager_ = Cast<ABuildManager>( found );
	}

	if ( !GridVisualizer_.IsValid() )
	{
		AActor* found = UGameplayStatics::GetActorOfClass( world, AGridVisualizer::StaticClass() );
		GridVisualizer_ = Cast<AGridVisualizer>( found );
	}

	if ( !GridManager_.IsValid() )
	{
		AActor* found = UGameplayStatics::GetActorOfClass( world, AGridManager::StaticClass() );
		GridManager_ = Cast<AGridManager>( found );
	}
}

void UCoreManager::FindPlayerControllerComponents()
{
	APlayerController* pc = GetPlayerController( 0 );
	if ( !pc )
	{
		return;
	}

	if ( !ResourceManager_.IsValid() )
	{
		ResourceManager_ = pc->FindComponentByClass<UResourceManager>();

		if ( !ResourceManager_.IsValid() )
		{
			UResourceManager* newRM = NewObject<UResourceManager>( pc, TEXT( "ResourceManager" ) );
			ResourceManager_ = newRM;
			newRM->RegisterComponent();
			UE_LOG( LogCoreManager, Log, TEXT( "Created ResourceManager on PlayerController" ) );
		}
	}

	if ( !EconomyComponent_.IsValid() )
	{
		EconomyComponent_ = pc->FindComponentByClass<UEconomyComponent>();

		if ( !EconomyComponent_.IsValid() )
		{
			UEconomyComponent* newEC = NewObject<UEconomyComponent>( pc, TEXT( "EconomyComponent" ) );
			EconomyComponent_ = newEC;
			newEC->RegisterComponent();
			UE_LOG( LogCoreManager, Log, TEXT( "Created EconomyComponent on PlayerController" ) );
		}
	}

	if ( !SelectionManager_.IsValid() )
	{
		SelectionManager_ = pc->FindComponentByClass<USelectionManagerComponent>();

		if ( !SelectionManager_.IsValid() )
		{
			USelectionManagerComponent* newEC = NewObject<USelectionManagerComponent>( pc, TEXT( "SelectionManager" ) );
			SelectionManager_ = newEC;
			newEC->RegisterComponent();
			UE_LOG( LogCoreManager, Log, TEXT( "Created SelectionManager on PlayerController" ) );
		}
	}
}

void UCoreManager::CreateInternalManagers()
{
	if ( !GameLoopManager_ )
	{
		GameLoopManager_ = NewObject<UGameLoopManager>( this, TEXT( "GameLoopManager" ) );
		UE_LOG( LogCoreManager, Log, TEXT( "Created GameLoopManager" ) );
	}
}

void UCoreManager::SetupManagerConnections()
{
	if ( !ResourceManager_.IsValid() )
	{
		UE_LOG( LogCoreManager, Error, TEXT( "SetupManagerConnections: ResourceManager is missing!" ) );
	}

	if ( !EconomyComponent_.IsValid() )
	{
		UE_LOG( LogCoreManager, Warning, TEXT( "SetupManagerConnections: EconomyComponent is missing!" ) );
	}

	if ( EconomyComponent_.IsValid() && ResourceManager_.IsValid() )
	{
		EconomyComponent_->SetResourceManager( ResourceManager_.Get() );
		UE_LOG( LogCoreManager, Log, TEXT( "Connected EconomyComponent -> ResourceManager" ) );
	}

	if ( GameLoopManager_ )
	{
		GameLoopManager_->Initialize( nullptr, WaveManager_.Get(), ResourceManager_.Get(), EconomyComponent_.Get() );
		UE_LOG( LogCoreManager, Log, TEXT( "GameLoopManager initialized (default config)" ) );
	}
	else
	{
		UE_LOG( LogCoreManager, Error, TEXT( "SetupManagerConnections: GameLoopManager is missing!" ) );
	}
}

void UCoreManager::LogSystemsStatus() const
{
#if !UE_BUILD_SHIPPING
	UE_LOG( LogCoreManager, Log, TEXT( "--- CoreManager Systems Status ---" ) );
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  WaveManager:      %s" ), WaveManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  BuildManager:     %s" ),
	    BuildManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  GridVisualizer:   %s" ),
	    GridVisualizer_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  GridManager:      %s" ), GridManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  ResourceManager:  %s [CRITICAL]" ),
	    ResourceManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  EconomyComponent: %s" ),
	    EconomyComponent_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  SelectionManager: %s (optional)" ),
	    SelectionManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG(
	    LogCoreManager, Log, TEXT( "  GameLoopManager:  %s [CRITICAL]" ),
	    GameLoopManager_ ? TEXT( "OK" ) : TEXT( "MISSING" )
	);
	UE_LOG( LogCoreManager, Log, TEXT( "----------------------------------" ) );
#endif
}
