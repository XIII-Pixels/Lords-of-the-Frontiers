// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Attack/AttackRangedComponent.h"

#include "Utilities/TraceChannelMappings.h"

UAttackRangedComponent::UAttackRangedComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    SightSphere_ = CreateDefaultSubobject<USphereComponent>( "Sphere Component" );
    SightSphere_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
    SightSphere_->SetCollisionObjectType( ECC_InvisibleVolume );
    SightSphere_->SetCollisionResponseToAllChannels( ECR_Ignore );
    SightSphere_->SetCollisionResponseToChannel( ECC_Entity, ECR_Overlap );
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
    if ( !Unit_ )
    {
        return;
    }

    EnemyInSight_ = nullptr;

    TArray<AActor*> overlappingActors;
    SightSphere_->GetOverlappingActors( overlappingActors, AActor::StaticClass() );

    TObjectPtr<AActor> target;
    float minDistance = -1.0f;
    for ( auto actor : overlappingActors )
    {
        if ( actor == GetOwner() )
        {
            continue;
        }

        auto enemy = Cast<IAttackable>( actor );
        if ( enemy && enemy->Team() != Unit_->Team() )
        {
            float dot = FVector::DotProduct(
                actor->GetActorLocation() - GetOwner()->GetActorLocation(),
                GetOwner()->GetActorForwardVector() );

            if ( dot > 0 && !target )
            {
                target = actor;
                minDistance = FVector::Distance( target->GetActorLocation(), actor->GetActorLocation() );
                continue;
            }

            if ( dot > 0 && target )
            {
                // UE_LOG( LogTemp, Log, TEXT( "Actor in sight: %s, (dot: %f)" ), *GetNameSafe( actor ), dot );
                float distance = FVector::Distance( target->GetActorLocation(), actor->GetActorLocation() );
                if ( distance < minDistance )
                {
                    target = actor;
                    minDistance = distance;
                }
            }
        }
    }

    EnemyInSight_ = target;
    GEngine->AddOnScreenDebugMessage(
        -1,
        1.0f,
        FColor::Magenta,
        FString::Printf( TEXT( "Enemy in sight: %s" ), *GetNameSafe( EnemyInSight_ ) )
    );
}
