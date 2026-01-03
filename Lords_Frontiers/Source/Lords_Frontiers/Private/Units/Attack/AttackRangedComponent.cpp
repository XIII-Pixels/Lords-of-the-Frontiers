// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Attack/AttackRangedComponent.h"

UAttackRangedComponent::UAttackRangedComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SightSphere_ = CreateDefaultSubobject<USphereComponent>( "Sphere Component" );
	SightSphere_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	SightSphere_->SetCollisionObjectType( ECC_EngineTraceChannel2 );
	SightSphere_->SetCollisionResponseToAllChannels( ECR_Ignore );
	SightSphere_->SetCollisionResponseToChannel( ECC_Pawn, ECR_Overlap );
	SightSphere_->SetGenerateOverlapEvents( true );
}

void UAttackRangedComponent::OnRegister()
{
	Super::OnRegister();

	Unit_ = Cast<AUnit>( GetOwner() );

	if ( Unit_ )
	{
		SightSphere_->SetupAttachment( Unit_->GetRootComponent() );
		SightSphere_->SetSphereRadius( Unit_->Stats().AttackRange() );
	}
}

void UAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&UAttackRangedComponent::SightTick,
		LookForwardTimeInterval_,
		true );
}

void UAttackRangedComponent::Attack(TObjectPtr<AActor> hitActor)
{
	Super::Attack( hitActor );
}

TObjectPtr<AActor> UAttackRangedComponent::EnemyInSight() const
{
	return EnemyInSight_;
}

void UAttackRangedComponent::SightTick()
{
	Look();

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&UAttackRangedComponent::SightTick,
		LookForwardTimeInterval_,
		true );
}

void UAttackRangedComponent::Look()
{
	TArray<AActor*> overlappingActors;
	SightSphere_->GetOverlappingActors( overlappingActors, AActor::StaticClass() );
	for ( auto actor : overlappingActors )
	{
		UE_LOG( LogTemp, Log, TEXT("Actor: %s"), *GetNameSafe(actor) )
	}
}
