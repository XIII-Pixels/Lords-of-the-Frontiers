#include "Tutorial/TutorialDirector.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Tutorial/TutorialConfig.h"
#include "Tutorial/TutorialSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC( LogTutorialDirector, Log, All );

ATutorialDirector::ATutorialDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATutorialDirector::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG( LogTutorialDirector, Log, TEXT( "BeginPlay, Config=%s" ), Config ? *Config->GetName() : TEXT( "null" ) );

	if ( !Config )
	{
		UE_LOG( LogTutorialDirector, Warning, TEXT( "Config not set on TutorialDirector; tutorial will not start" ) );
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	if ( StartDelay <= 0.0f )
	{
		StartNow();
		return;
	}

	world->GetTimerManager().SetTimer( StartTimerHandle_, this, &ATutorialDirector::StartNow, StartDelay, false );
}

void ATutorialDirector::EndPlay( const EEndPlayReason::Type reason )
{
	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( StartTimerHandle_ );
	}

	if ( UTutorialSubsystem* sub = UTutorialSubsystem::Get( this ) )
	{
		sub->StopTutorial();
	}

	Super::EndPlay( reason );
}

void ATutorialDirector::StartNow()
{
	if ( UTutorialSubsystem* sub = UTutorialSubsystem::Get( this ) )
	{
		sub->StartTutorial( Config );
	}
}
