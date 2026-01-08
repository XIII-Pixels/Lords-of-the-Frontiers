#include "Lords_Frontiers/Public/SimpleEnemy.h"
#include "Lords_Frontiers/Public/EntityStats.h"
#include "Components/StaticMeshComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

// this wont be included in actual build

ASimpleEnemy::ASimpleEnemy () 
{
	PrimaryActorTick.bCanEverTick = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent> ( TEXT ( "Capsule" ) );
	CapsuleComponent->InitCapsuleSize ( DefaultCapsuleRadius, DefaultCapsuleHalfHeight ); // debug size
	CapsuleComponent->SetCollisionProfileName ( TEXT ( "Pawn" ) );
	SetRootComponent ( CapsuleComponent );

	// Create mesh component and set it as root so spawned enemy is visible
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent> ( TEXT ( "MeshComponent" ) );
	MeshComponent->SetupAttachment ( CapsuleComponent );
	MeshComponent->SetRelativeLocation ( DefaultMeshRelativeLocation );
	MeshComponent->SetCollisionEnabled ( ECollisionEnabled::QueryAndPhysics );
	MeshComponent->SetCollisionProfileName ( TEXT ( "Pawn" ) );

	// sensible defaults
	MaxHealth = 100;
	Health = MaxHealth;
	AttackDamage = 10;

	// Let engine spawn a default AI controller ( can be overridden in Blueprint ) 
	AIControllerClass = AAIController::StaticClass ();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

void ASimpleEnemy::BeginPlay () 
{
	Super::BeginPlay ();


#if WITH_EDITOR
	// Draw a small debug sphere so you can see the spawn in editor/game for a second
	if ( GetWorld () ) 
	{
		DrawDebugSphere ( GetWorld () , GetActorLocation () , 24.0f, 8, FColor::Red, false, 2.0f );
	}
#endif
}

void ASimpleEnemy::TakeDamage ( float DamageAmount ) 
{
	// Basic guard
	if ( DamageAmount <= 0.0f || Health <= 0 ) 
	{
		return;
	}

	Health = FMath::Clamp ( Health - DamageAmount, 0.0f, MaxHealth );

	// Debug log
	if ( GEngine ) 
	{
		GEngine->AddOnScreenDebugMessage ( -1, 2.0f, FColor::Yellow,
			FString::Printf ( TEXT ( "%s took %.1f damage, HP=%.1f/%.1f" ) , *GetName () , DamageAmount, Health, MaxHealth ) );
	}

	// If died -> handle death
	if ( Health <= 0.0f ) 
	{
		HandleDeath ();
	}
}

void ASimpleEnemy::HandleDeath () 
{
	// Debug
	UE_LOG ( LogTemp, Log, TEXT ( "%s died." ) , *GetName () );

	// Optional: spawn VFX or sound here

	// Delay destroy slightly to allow any effects / bindings to execute
	if ( GetWorld () ) 
	{
		FTimerHandle Th;
		GetWorld () ->GetTimerManager () .SetTimer ( Th, [this] () 
			{
				Destroy ();
			}, 0.1f, false );
	}
	else
	{
		Destroy ();
	}
}

