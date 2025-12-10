#include "Lords_Frontiers/Public/Building/MainBase.h"

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