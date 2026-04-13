#include "Lords_Frontiers/Public/Building/MainBase.h"
#include "Core/GameSessionController.h"
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

	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		UE_LOG( LogTemp, Log, TEXT( "AMainBase::OnDeath - Calling SessionController::EndGame(Lose)." ) );
		session->EndGame( EGameResult::Lose );
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "AMainBase::OnDeath - GameSessionController not found." ) );
	}
}