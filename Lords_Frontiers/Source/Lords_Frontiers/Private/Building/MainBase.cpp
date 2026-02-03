#include "Lords_Frontiers/Public/Building/MainBase.h"
#include "Lords_Frontiers/Public/Core/GameLoopManager.h"
#include "Lords_Frontiers/Public/Core/CoreManager.h"
#include "Kismet/GameplayStatics.h"

AMainBase::AMainBase()
{
	// Set specific stats for the Main Base
	Stats_.SetMaxHealth( cBaseMaxHealth );
	Stats_.SetHealth( cBaseMaxHealth );
}

void AMainBase::BeginPlay()
{
	Super::BeginPlay();

	// Logic for registering the base in GameManager can be added here
}

void AMainBase::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	Super::EndPlay( endPlayReason );

	// Logic for Game Over if destroyed (EndPlayReason == Destroyed)
}

void AMainBase::OnDeath()
{
	Super::OnDeath();

	if ( bDefeatHandled )
	{
		return;
	}
	bDefeatHandled = true;

	UE_LOG( LogTemp, Log, TEXT( "AMainBase::OnDeath - main base died, attempting to trigger defeat phase." ) );

	UCoreManager* core = UCoreManager::Get( this );

	if ( core )
	{
		UGameLoopManager* GameLoop = core->GetGameLoop();
		if ( GameLoop )
		{
			UE_LOG(
			    LogTemp, Log,
			    TEXT( "AMainBase::OnDeath - Found CoreManager (%p) and GameLoop (%p). Calling EnterDefeatPhase()." ),
			    core, GameLoop
			);
			GameLoop->EnterDefeatPhase();
			return;
		}
		else
		{
			UE_LOG( LogTemp, Warning, TEXT( "AMainBase::OnDeath - CoreManager found but GameLoop is null." ) );
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "AMainBase::OnDeath - CoreManager not found via UCoreManager::Get(this)." ) );
	}

	/* 2) Fallback: if Core/GameLoop not found — open level from here.
	UWorld* World = GetWorld();
	if ( World )
	{
		UE_LOG( LogTemp, Warning, TEXT( "AMainBase::OnDeath - Falling back to OpenLevel(\"Lose\")." ) );
		UGameplayStatics::OpenLevel( World, FName( TEXT( "Lose" ) ) );
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "AMainBase::OnDeath - No World available to open Lose level fallback." ) );
	}
	Super::OnDeath();*/
}