// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Units/UnitMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Transform/TransformableHandleUtils.h"
#include "Units/UnitAIController.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionProfileName( TEXT( "Pawn" ) );

	MovementComponent_ = CreateDefaultSubobject<UUnitMovementComponent>( TEXT( "UnitMovementComponent" ) );
	MovementComponent_->UpdatedComponent = CollisionComponent_;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass_ = AUnitAIController::StaticClass();
	AIControllerClass = UnitAIControllerClass_;
}

void AUnit::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction( transform );
	AIControllerClass = UnitAIControllerClass_;
}

void AUnit::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&AUnit::SightTick,
		SightRaycastInterval_,
		true );
}

void AUnit::Tick(float deltaSeconds)
{
	Super::Tick( deltaSeconds );
}

void AUnit::Attack(TObjectPtr<AActor> hitActor)
{
	// Probably should be done with some attack component, because some buildings can attack as well
	// Probably should use some attack manager, because it would be easier to fetch attack info

	if ( Stats_.OnCooldown() )
	{
		return;
	}
	
	auto attacked = Cast<IAttackable>(hitActor);
	if ( !attacked )
	{
		return;
	}

	attacked->TakeDamage( Stats_.GetAttackDamage() );
	Stats_.StartCooldown();

	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Emerald, "Damage applied" );
}

void AUnit::TakeDamage(float damage)
{
	Stats_.ApplyDamage( damage );
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, "Damage taken" );
}

TObjectPtr<AActor> AUnit::EnemyInSight() const
{
	return EnemyInSight_;
}

const TObjectPtr<UBehaviorTree>& AUnit::BehaviorTree() const
{
	return UnitBehaviorTree_;
}

const TObjectPtr<AActor>& AUnit::Target() const
{
	return FollowedTarget_;
}

void AUnit::SightTick()
{
	LookForward();

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&AUnit::SightTick,
		SightRaycastInterval_,
		true );
}

void AUnit::LookForward()
{
	FVector start = GetActorLocation();
	FVector end = start + GetActorForwardVector() * Stats_.GetAttackRange();

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( this );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Pawn, params ) )
	{
		AActor* hitActor = hit.GetActor();
		GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Blue, "Actor seen" );
		if ( hitActor->GetClass()->ImplementsInterface( UAttackable::StaticClass() ) )
		{
			GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, "Enemy seen" );
			EnemyInSight_ = hitActor;
			return;
		}
	}

	EnemyInSight_ = nullptr;
}
