// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Units/FollowComponent.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Transform/TransformableHandleUtils.h"
#include "Units/UnitAIController.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionProfileName( TEXT( "Pawn" ) );

	FollowComponent_ = CreateDefaultSubobject<UFollowComponent>( TEXT( "UnitMovementComponent" ) );
	FollowComponent_->UpdatedComponent = CollisionComponent_;

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
		LookForwardTimeInterval_,
		true );
}

void AUnit::Tick(float deltaSeconds)
{
	Super::Tick( deltaSeconds );
}

void AUnit::StartFollowing()
{
	FollowComponent_->StartFollowing();
}

void AUnit::StopFollowing()
{
	FollowComponent_->StopFollowing();
}

void AUnit::Attack(TObjectPtr<AActor> hitActor)
{
	// Probably should be done with some attack component, because some buildings can attack as well
	// Probably should use some attack manager, because it would be easier to fetch attack info

	if ( Stats_.OnCooldown() )
	{
		return;
	}

	auto attacked = Cast<IAttackable>( hitActor );
	if ( !attacked )
	{
		return;
	}

	if ( Stats_.Team() == attacked->Team() )
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Emerald, "Damage applied" );

	attacked->TakeDamage( Stats_.AttackDamage() );
	Stats_.StartCooldown();
}

void AUnit::TakeDamage(float damage)
{
	Stats_.ApplyDamage( damage );
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Emerald, "Damage taken" );

	if ( !Stats_.IsAlive() )
	{
		GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Emerald, "DEAD!" );
	}
}

ETeam AUnit::Team()
{
	return Stats_.Team();
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
	return FollowComponent_->Target();
}

void AUnit::SightTick()
{
	LookForward();

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&AUnit::SightTick,
		LookForwardTimeInterval_,
		true );
}

void AUnit::LookForward()
{
	FVector start = GetActorLocation();
	FVector end = start + GetActorForwardVector() * Stats_.AttackRange();

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( this );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Pawn, params ) )
	{
		AActor* hitActor = hit.GetActor();
		if ( auto hitEntity = Cast<IAttackable>( hitActor ) )
		{
			if ( Stats_.Team() != hitEntity->Team() )
			{
				EnemyInSight_ = hitActor;
				GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, "Enemy seen" );
				return;
			}
			GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Blue, "Ally seen" );
			return;
		}
		GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, "Actor seen" );
	}

	EnemyInSight_ = nullptr;
}
