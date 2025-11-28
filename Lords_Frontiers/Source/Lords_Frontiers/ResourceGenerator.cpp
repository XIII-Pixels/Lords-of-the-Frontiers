#include "ResourceGenerator.h"
#include "ResourceManager.h"
#include "Kismet/GameplayStatics.h"

AResourceGenerator::AResourceGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	ResourceType_ = EResourceType::Gold;
	GenerationQuantity_ = 10;
	GenerationInterval_ = cDefaultInterval;
	ResourceManager_ = nullptr;
}

void AResourceGenerator::BeginPlay()
{
	Super::BeginPlay();

	FindResourceManager_();
	StartGeneration();
}

void AResourceGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopGeneration();
	Super::EndPlay(EndPlayReason);
}

void AResourceGenerator::StartGeneration()
{
	UWorld* World = GetWorld();
	if ( IsValid(World) && (GenerationInterval_ > 0.0f) )
	{
		World->GetTimerManager().SetTimer(
			GenerationTimerHandle_,
			this,
			&AResourceGenerator::Generate_,
			GenerationInterval_,
			true
		);
	}
}

void AResourceGenerator::StopGeneration()
{
	UWorld* World = GetWorld();
	if ( IsValid(World) )
	{
		World->GetTimerManager().ClearTimer(GenerationTimerHandle_);
	}
}

void AResourceGenerator::Generate_()
{
	if ( !IsValid(ResourceManager_) )
	{
		// Repeat the search if the link is lost or was not found at the start.
		FindResourceManager_();
	}

	if ( IsValid(ResourceManager_) )
	{
		ResourceManager_->AddResource(ResourceType_, GenerationQuantity_);
	}
}

void AResourceGenerator::FindResourceManager_()
{
	// We are looking for a component from the first player's PlayerPawn or PlayerController
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if ( IsValid(PlayerPawn) )
	{
		ResourceManager_ = PlayerPawn->FindComponentByClass<UResourceManager>();
	}

	if ( !IsValid(ResourceManager_) )
	{
		// Search in the controller if there is no pawn
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if ( IsValid(PC) )
		{
			ResourceManager_ = PC->FindComponentByClass<UResourceManager>();
		}
	}
}