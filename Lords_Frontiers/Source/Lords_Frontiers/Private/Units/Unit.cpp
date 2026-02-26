// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "AI/EntityAIController.h"
#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/Path/PathTargetPoint.h"
#include "Transform/TransformableHandleUtils.h"
#include "Utilities/TraceChannelMappings.h"
#include "Lords_Frontiers/Public/UI/HealthBarManager.h"
#include "Waves/EnemyBuff.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/FollowComponent.h"
#include "UI/Widgets/BuildingTooltipWidget.h"
#include "UI/Widgets/HealthBarWidget.h"
#include "Kismet/GameplayStatics.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionObjectType( ECC_Entity );

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass_ = AEntityAIController::StaticClass();
}

void AUnit::OnConstruction( const FTransform& transform )
{
	Super::OnConstruction( transform );
	AIControllerClass = UnitAIControllerClass_;
}

void AUnit::BeginPlay()
{
	Super::BeginPlay();

	Stats_.SetHealth( Stats_.MaxHealth() );

	FollowComponent_ = FindComponentByClass<UFollowComponent>();
	if ( FollowComponent_ )
	{
		FollowComponent_->SetMaxSpeed( Stats_.MaxSpeed() );
		FollowComponent_->UpdatedComponent = CollisionComponent_;
	}
	CachedHealthBarManager_.Reset();

	UWorld* World = GetWorld();
	if ( !World )
	{
		UE_LOG( LogTemp, Warning, TEXT( "AUnit::BeginPlay: GetWorld() == nullptr" ) );
		return;
	}

	AActor* Found = UGameplayStatics::GetActorOfClass( World, AHealthBarManager::StaticClass() );
	UE_LOG(
	    LogTemp, Log, TEXT( "AUnit::BeginPlay: GetActorOfClass(AHealthBarManager) -> ptr=%p name=%s class=%s" ), Found,
	    *GetNameSafe( Found ), Found ? *Found->GetClass()->GetName() : TEXT( "null" )
	);

	if ( Found )
	{
		if ( AHealthBarManager* Mgr = Cast<AHealthBarManager>( Found ) )
		{
			CachedHealthBarManager_ = Mgr;
			UE_LOG( LogTemp, Log, TEXT( "AUnit::BeginPlay: Cached HealthBarManager = %s" ), *GetNameSafe( Mgr ) );
		}
		else
		{
			UE_LOG(
			    LogTemp, Error,
			    TEXT( "AUnit::BeginPlay: Found actor is NOT AHealthBarManager, ptr=%p class=%s name=%s" ), Found,
			    *GetNameSafe( Found->GetClass() ), *GetNameSafe( Found )
			);
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "AUnit::BeginPlay: HealthBarManager not found in level" ) );
	}
	if ( !HealthWidgetComponent )
	{
		HealthWidgetComponent = FindComponentByClass<UWidgetComponent>();
	}

	if ( !HealthWidgetComponent )
	{
		HealthWidgetComponent = NewObject<UWidgetComponent>( this, TEXT( "HealthWidgetComponent" ) );
		if ( HealthWidgetComponent )
		{
			HealthWidgetComponent->RegisterComponent();
			HealthWidgetComponent->AttachToComponent(
			    GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform
			);
		}
	}

	if ( HealthWidgetComponent )
	{
		HealthWidgetComponent->SetWidgetSpace( EWidgetSpace::Screen );
		HealthWidgetComponent->SetRelativeLocation( HealthBarWorldOffset );
		HealthWidgetComponent->SetPivot( FVector2D( 0.5f, 1.0f ) );       
		HealthWidgetComponent->SetDrawSize( FIntPoint( 220, 28 ) );     

		if ( HealthBarWidgetClass )
		{
			HealthWidgetComponent->SetWidgetClass( HealthBarWidgetClass );
		}

		HealthWidgetComponent->InitWidget();

		if ( UUserWidget* UW = HealthWidgetComponent->GetUserWidgetObject() )
		{
			if ( UHealthBarWidget* HBW = Cast<UHealthBarWidget>( UW ) )
			{
				HBW->BindToActor( this );
				HBW->SetVisibility( ESlateVisibility::Collapsed );
			}
		}
	}

	TArray<UAttackComponent*> attackComponents;
	GetComponents( attackComponents );

	if ( attackComponents.Num() == 1 )
	{
		AttackComponent_ = attackComponents[0];
	}
	else
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "Unit: number of unit attack components is not equal to 1 (number: %d)" ),
		    attackComponents.Num()
		);
	}

	SwayPhaseOffset_ = FMath::FRandRange( 0.0f, 6.28f );

	VisualMesh_ = Cast<USceneComponent>( GetComponentByClass( UMeshComponent::StaticClass() ) );
}

void AUnit::Tick( float deltaSeconds )
{
	Super::Tick( deltaSeconds );

	if ( FollowedTarget_.Get()->IsA( APathTargetPoint::StaticClass() ) && IsCloseToTarget() )
	{
		FollowNextPathTarget();
	}

	if ( VisualMesh_ )
	{
		float targetRoll = 0.0f;

		if ( GetVelocity().Size() > 10.0f )
		{
			float time = GetWorld()->GetTimeSeconds();
			targetRoll = FMath::Sin( time * SwaySpeed_ + SwayPhaseOffset_ ) * SwayAmplitude_;
		}

		CurrentSwayRoll_ = FMath::FInterpTo( CurrentSwayRoll_, targetRoll, deltaSeconds, 10.0f );

		FRotator currentRot = VisualMesh_->GetRelativeRotation();
		currentRot.Pitch = CurrentSwayRoll_;
		currentRot.Roll = 0.0f;
		VisualMesh_->SetRelativeRotation( currentRot );
	}
}

void AUnit::StartFollowing()
{
	if ( FollowComponent_ )
	{
		FollowComponent_->StartFollowing();
	}
}

void AUnit::StopFollowing()
{
	if ( FollowComponent_ )
	{
		FollowComponent_->StopFollowing();
	}
}

void AUnit::Attack( TObjectPtr<AActor> hitActor )
{
	if ( AttackComponent_ )
	{
		AttackComponent_->Attack( hitActor );
	}
}

void AUnit::TakeDamage( float damage )
{
	if ( !Stats_.IsAlive() )
	{
		return;
	}

	Stats_.ApplyDamage( damage );
	if ( AHealthBarManager* manager = Cast<AHealthBarManager>( UGameplayStatics::GetActorOfClass( GetWorld(), AHealthBarManager::StaticClass() ) ) )
	{
		manager->OnActorHealthChanged( this, this->GetCurrentHealth(), this->GetMaxHealth() );
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "AUnit::TakeDamage: HealthBarManager NOT FOUND!" ) );
	}
	//OnUnitHealthChanged.Broadcast( this->GetCurrentHealth(), this->GetMaxHealth() );
	if ( !Stats_.IsAlive() )
	{
		OnDeath();
	}

}

FEntityStats& AUnit::Stats()
{
	return Stats_;
}

ETeam AUnit::Team()
{
	return Stats_.Team();
}

TObjectPtr<AActor> AUnit::EnemyInSight() const
{
	if ( AttackComponent_ )
	{
		return AttackComponent_->EnemyInSight();
	}
	return nullptr;
}

TObjectPtr<UBehaviorTree> AUnit::BehaviorTree() const
{
	return UnitBehaviorTree_;
}

TWeakObjectPtr<AActor> AUnit::FollowedTarget() const
{
	return FollowedTarget_;
}

const TObjectPtr<UPath>& AUnit::Path() const
{
	return Path_;
}

void AUnit::SetFollowedTarget( TObjectPtr<AActor> followedTarget )
{
	FollowedTarget_ = followedTarget;
}

void AUnit::OnDeath()
{
	// When HP becomes 0

	if ( AttackComponent_ )
	{
		AttackComponent_->DeactivateSight();
	}

	if ( FollowComponent_ )
	{
		FollowComponent_->Deactivate();
	}

	Destroy();
}

void AUnit::FollowNextPathTarget()
{
	AdvancePathPointIndex();
	FollowPath();
}

bool AUnit::IsCloseToTarget() const
{
	if ( !FollowedTarget_.IsValid() || !PathPointsManager_.IsValid() )
	{
		return false;
	}

	const float distanceSq = FVector::DistSquared( GetActorLocation(), FollowedTarget_->GetActorLocation() );
	const float radiusSq = PathPointsManager_->PointReachRadius * PathPointsManager_->PointReachRadius;
	return distanceSq < radiusSq;
}

void AUnit::SetPath( TObjectPtr<UPath> path )
{
	Path_ = path;
	PathPointIndex_ = 0;
}

void AUnit::SetPathPointsManager( TWeakObjectPtr<APathPointsManager> pathPointsManager )
{
	PathPointsManager_ = pathPointsManager;
}

void AUnit::AdvancePathPointIndex()
{
	++PathPointIndex_;
}

void AUnit::SetPathPointIndex( int pathPointIndex )
{
	PathPointIndex_ = pathPointIndex;
}

void AUnit::FollowPath()
{
	if ( !Path_ )
	{
		UE_LOG( LogTemp, Error, TEXT( "Unit: no valid Path_. Cannot follow path" ) );
		FollowedTarget_ = nullptr;
		return;
	}

	if ( !PathPointsManager_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "Unit: no valid PathPointsManager_. Cannot follow path" ) );
		FollowedTarget_ = nullptr;
		return;
	}

	const TArray<FIntPoint>& pathPoints = Path_->GetPoints();
	if ( 0 > PathPointIndex_ || PathPointIndex_ >= pathPoints.Num() )
	{
		if ( PathPointsManager_->GoalActor.IsValid() )
		{
			FollowedTarget_ = PathPointsManager_->GoalActor;
		}
		else
		{
			UE_LOG( LogTemp, Error, TEXT( "Unit: PathPointIndex_ is out of range and no GoalActor specified" ) );
			FollowedTarget_ = nullptr;
		}
	}
	else
	{
		FollowedTarget_ = PathPointsManager_->GetTargetPoint( pathPoints[PathPointIndex_] ).Get();
	}
}

void AUnit::SetFollowedTarget( AActor* newTarget )
{
	FollowedTarget_ = newTarget;
}

int AUnit::GetCurrentHealth() const
{
	return Stats_.Health();
}
int AUnit::GetMaxHealth() const
{
	return Stats_.MaxHealth();
}


void AUnit::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if ( HealthWidgetComponent )
	{
		if ( UUserWidget* UW = HealthWidgetComponent->GetUserWidgetObject() )
		{
			if ( UHealthBarWidget* HBW = Cast<UHealthBarWidget>( UW ) )
			{
				HBW->Unbind( nullptr );
			}
		}
	}

	Super::EndPlay( EndPlayReason );
}
void AUnit::ChangeStats( FEnemyBuff* buff )
{
	Stats_.SetMaxHealth(
	    FMath::FloorToInt( Stats_.MaxHealth() * FMath::Pow( buff->HealthMultiplier, buff->SpawnCount ) )
	);
	Stats_.SetAttackRange( Stats_.AttackRange() * FMath::Pow( buff->AttackRangeMultiplier, buff->SpawnCount ) );
	Stats_.SetAttackDamage(
	    FMath::FloorToInt( Stats_.AttackDamage() * FMath::Pow( buff->AttackDamageMultiplier, buff->SpawnCount ) )
	);
	Stats_.SetAttackCooldown(
	    Stats_.AttackCooldown() * FMath::Pow( buff->AttackCooldownMultiplier, buff->SpawnCount )
	);
	Stats_.SetMaxSpeed( Stats_.MaxSpeed() * FMath::Pow( buff->MaxSpeedMultiplier, buff->SpawnCount ) );
	Stats_.Heal( Stats_.MaxHealth() );

}
