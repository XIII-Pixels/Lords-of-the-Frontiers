// MainGameMode.cpp
#include "Core/GameModes/MainGameMode.h"

#include "Cards/CardPoolConfig.h"
#include "Cards/CardSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Grid/GridManager.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

AMainGameMode::AMainGameMode()
{
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	SetupCamera();
	InitializeGameSystems();
	InitializeCardSystem();
	CreateHUD();
}

void AMainGameMode::InitializeGameSystems()
{
	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
	{
		UE_LOG( LogTemp, Error, TEXT( "MainGameMode: CoreManager not found!" ) );
		return;
	}

	Core->InitializeSystems();
	Core->RefreshSystemReferences();

	UGameLoopManager* GL = Core->GetGameLoop();
	if ( !GL )
	{
		UE_LOG( LogTemp, Error, TEXT( "MainGameMode: GameLoopManager not found!" ) );
		return;
	}

	if ( GameLoopConfig )
	{
		GL->Initialize(
		    GameLoopConfig, Core->GetWaveManager(), Core->GetResourceManager(), Core->GetEconomyComponent()
		);
		UE_LOG( LogTemp, Log, TEXT( "MainGameMode: GameLoop initialized with config" ) );
	}
	GL->StartGame();
	UE_LOG( LogTemp, Log, TEXT( "MainGameMode: Game started" ) );
}

void AMainGameMode::InitializeCardSystem()
{
	UCardSubsystem* cardSystem = UCardSubsystem::Get( this );
	if ( !cardSystem )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MainGameMode: CardSubsystem not found!" ) );
		return;
	}

	if ( CardPoolConfig )
	{
		cardSystem->SetPoolConfig( CardPoolConfig );
		UE_LOG(
		    LogTemp, Log, TEXT( "MainGameMode: Card system initialized with %d cards" ), CardPoolConfig->GetPoolSize()
		);
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "MainGameMode: CardPoolConfig not set!" ) );
	}
}

void AMainGameMode::CreateHUD()
{
	if ( !HUDWidgetClass )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MainGameMode: HUDWidgetClass not set" ) );
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController( this, 0 );
	if ( !PC )
	{
		return;
	}

	HUDWidget = CreateWidget<UUserWidget>( PC, HUDWidgetClass );
	if ( HUDWidget )
	{
		HUDWidget->AddToViewport();
		UE_LOG( LogTemp, Log, TEXT( "MainGameMode: HUD created" ) );
	}
}

void AMainGameMode::SetupCamera()
{
	UWorld* World = GetWorld();
	if ( !World )
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if ( !PC )
	{
		return;
	}

	ACameraActor* Camera =
	    Cast<ACameraActor>( UGameplayStatics::GetActorOfClass( World, ACameraActor::StaticClass() ) );

	if ( !Camera )
	{
		FVector GridCenter = FVector::ZeroVector;
		float CellSize = 100.0f;
		int32 Width = 10;
		int32 Height = 10;

		if ( AGridManager* Grid =
		         Cast<AGridManager>( UGameplayStatics::GetActorOfClass( World, AGridManager::StaticClass() ) ) )
		{
			GridCenter = Grid->GetActorLocation();
			CellSize = Grid->GetCellSize();
			Width = Grid->GetGridWidth();
			Height = Grid->GetGridHeight();

			GridCenter.X += Width * CellSize * 0.5f;
			GridCenter.Y += Height * CellSize * 0.5f;
		}

		const float Distance = 1500.0f;
		const float HeightZ = 1500.0f;

		FVector CameraLocation = GridCenter + FVector( -Distance, -Distance, HeightZ );
		FRotator CameraRotation = ( GridCenter - CameraLocation ).Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Camera = World->SpawnActor<ACameraActor>( CameraLocation, CameraRotation, SpawnParams );
	}

	if ( Camera )
	{
		PC->SetViewTarget( Camera );
	}
}