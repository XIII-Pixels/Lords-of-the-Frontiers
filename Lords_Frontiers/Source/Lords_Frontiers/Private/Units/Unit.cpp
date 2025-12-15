// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Units/UnitMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Units/UnitAIController.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionProfileName( TEXT( "Pawn" ) );

	MovementComponent_ = CreateDefaultSubobject<UUnitMovementComponent>( TEXT( "UnitMovementComponent" ) );
	MovementComponent_->UpdatedComponent = CollisionComponent_;

	AutoPossessAI         = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass = AUnitAIController::StaticClass();
	AIControllerClass     = UnitAIControllerClass;
}

void AUnit::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction( transform );
	AIControllerClass = UnitAIControllerClass;
}

void AUnit::Tick(float deltaSeconds)
{
	Super::Tick( deltaSeconds );

	static float time = 0.0f;
	if ((time += deltaSeconds) > 1.0f)
	{
		AttackForward();
		time = 0.0f;
	}
}

void AUnit::AttackForward()
{
	FVector start = GetActorLocation();
	FVector end   = start + GetActorForwardVector() * Stats.AttackRange;

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( this );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Pawn, params ) )
	{
		AActor* hitActor = hit.GetActor();
		GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Blue, "Hit" );
		if ( hitActor->GetClass()->ImplementsInterface( UAttackable::StaticClass() ) )
		{
			DealDamage( hitActor );
		}
	}
}

void AUnit::DealDamage(TScriptInterface<IAttackable> target)
{
	// Probably should be done with some attack component, because some buildings can attack as well
	// Probably should use some attack manager, because it would be easier to fetch attack info
	target->TakeDamage( Stats.AttackDamage );
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, "Damage" );
}

void AUnit::TakeDamage(float damage)
{
	// Stats.HP -= damage
}

const TObjectPtr<UBehaviorTree>& AUnit::BehaviorTree() const
{
	return UnitBehaviorTree;
}

const TObjectPtr<AActor>& AUnit::Target() const
{
	return FollowedTarget;
}
