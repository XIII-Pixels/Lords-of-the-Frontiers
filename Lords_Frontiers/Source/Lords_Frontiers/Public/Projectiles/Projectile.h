// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.generated.h"

class USphereComponent;

/** (Gregory-hub)
 * Follows target and damages it upon hit */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();

    void Initialize(TObjectPtr<AActor> target, float damage, float speed, bool splash = false);

    void Launch();

    virtual void Tick(float deltaTime) override;

protected:
    void DealDamage(TObjectPtr<AActor> target) const;

    UFUNCTION()
    void OnCollisionStart(UPrimitiveComponent* overlappedComp,
        AActor* otherActor,
        UPrimitiveComponent* otherComp,
        int32 otherBodyIndex,
        bool bFromSweep,
        const FHitResult& sweepResult);

    // UFUNCTION()
    // void OnCollisionEnd(UPrimitiveComponent* overlappedComp, AActor* otherActor, 
    //                                 UPrimitiveComponent* otherComp, int32 otherBodyIndex);

    UPROPERTY()
    TObjectPtr<USphereComponent> CollisionComponent_;

    UPROPERTY()
    TObjectPtr<AActor> Target_;

    float Damage_ = 0.0f;

    float Speed_ = 0.0f;

    bool bSplash_ = false;

    bool bIsFollowing_ = false;
};
