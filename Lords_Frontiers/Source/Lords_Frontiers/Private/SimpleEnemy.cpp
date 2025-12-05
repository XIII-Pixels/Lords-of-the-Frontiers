#include "Lords_Frontiers/Public/SimpleEnemy.h"
#include "Lords_Frontiers/Public/EntityStats.h"
#include "Components/StaticMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ASimpleEnemy::ASimpleEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->InitCapsuleSize(34.f, 88.f); // пример размеров
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(CapsuleComponent);

	// Create mesh component and set it as root so spawned enemy is visible
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CapsuleComponent);
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -88.f)); // подогнать, если нужно
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionProfileName(TEXT("Pawn"));

	// sensible defaults
	MaxHealth = 100;
	Health = MaxHealth;
	AttackDamage = 10;

	// Let engine spawn a default AI controller (can be overridden in Blueprint)
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

void ASimpleEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Ensure health consistent with MaxHealth
	Health = FMath::Clamp(Health, 0, MaxHealth);

#if WITH_EDITOR
	// Draw a small debug sphere so you can see the spawn in editor/game for a second
	if (GetWorld())
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 24.0f, 8, FColor::Red, false, 2.0f);
	}
#endif
}

void ASimpleEnemy::TakeDamage(float DamageAmount)
{
	// Basic guard
	if (DamageAmount <= 0.0f || Health <= 0)
	{
		return;
	}

	// convert to int damage (round) and apply
	const int32 IntDamage = FMath::Max(0, FMath::RoundToInt(DamageAmount));
	Health = FMath::Clamp(Health - IntDamage, 0, MaxHealth);

	// Debug log
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("%s took %d damage, HP=%d/%d"), *GetName(), IntDamage, Health, MaxHealth));
	}

	// If died -> handle death
	if (Health <= 0)
	{
		HandleDeath();
	}
}

void ASimpleEnemy::HandleDeath()
{
	// Debug
	UE_LOG(LogTemp, Log, TEXT("%s died."), *GetName());

	// Optional: spawn VFX or sound here

	// Delay destroy slightly to allow any effects / bindings to execute
	if (GetWorld())
	{
		FTimerHandle Th;
		GetWorld()->GetTimerManager().SetTimer(Th, [this]()
			{
				Destroy();
			}, 0.1f, false);
	}
	else
	{
		Destroy();
	}
}

