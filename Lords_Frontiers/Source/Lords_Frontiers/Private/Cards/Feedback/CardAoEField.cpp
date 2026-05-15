#include "Cards/Feedback/CardAoEField.h"

#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Cards/Visuals/CardAoEDebug.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

ACardAoEField::ACardAoEField()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot_ = CreateDefaultSubobject<USceneComponent>( TEXT( "SceneRoot" ) );
	RootComponent = SceneRoot_;
}

void ACardAoEField::Initialize(
	AActor* instigator,
	float radius,
	float duration,
	float tickInterval,
	int32 damagePerTick,
	UStatusEffectDef* statusPerTick,
	bool bDebugDrawRadius,
	const FColor& debugColor )
{
	Instigator_ = instigator;
	Radius_ = FMath::Max( 0.f, radius );
	Duration_ = FMath::Max( 0.05f, duration );
	TickInterval_ = FMath::Max( 0.05f, tickInterval );
	DamagePerTick_ = damagePerTick;
	StatusPerTick_ = statusPerTick;
	bDebugDrawRadius_ = bDebugDrawRadius;
	DebugColor_ = debugColor;
	Elapsed_ = 0.f;
	NextTickAt_ = TickInterval_;

	ApplyVisualScale();
}

void ACardAoEField::ApplyVisualScale()
{
	if ( !bScaleVisualToRadius || !SceneRoot_ || Radius_ <= 0.f )
	{
		return;
	}

	const float reference = FMath::Max( 1.f, VisualReferenceRadius );
	const float factor = Radius_ / reference;
	SceneRoot_->SetWorldScale3D( FVector( factor ) );
}

void ACardAoEField::BeginPlay()
{
	Super::BeginPlay();

	if ( const UWorld* world = GetWorld() )
	{
		if ( UGameInstance* gameInstance = world->GetGameInstance() )
		{
			if ( UGameLoopManager* loop = gameInstance->GetSubsystem<UGameLoopManager>() )
			{
				loop->OnPhaseChanged.AddDynamic( this, &ACardAoEField::HandleGameLoopPhaseChanged );
			}
		}
	}
}

void ACardAoEField::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	if ( const UWorld* world = GetWorld() )
	{
		if ( UGameInstance* gameInstance = world->GetGameInstance() )
		{
			if ( UGameLoopManager* loop = gameInstance->GetSubsystem<UGameLoopManager>() )
			{
				loop->OnPhaseChanged.RemoveDynamic( this, &ACardAoEField::HandleGameLoopPhaseChanged );
			}
		}
	}

	Super::EndPlay( endPlayReason );
}

void ACardAoEField::HandleGameLoopPhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	if ( oldPhase == EGameLoopPhase::Combat && newPhase != EGameLoopPhase::Combat )
	{
		Destroy();
	}
}

void ACardAoEField::Tick( float deltaTime )
{
	Super::Tick( deltaTime );

	Elapsed_ += deltaTime;

	if ( bDebugDrawRadius_ )
	{
		CardAoEDebug::DrawRadius( this, GetActorLocation(), Radius_, 0.f, DebugColor_ );
	}

	if ( Elapsed_ >= NextTickAt_ )
	{
		ApplyToEnemies();
		NextTickAt_ += TickInterval_;
	}

	if ( Elapsed_ >= Duration_ )
	{
		Destroy();
	}
}

void ACardAoEField::ApplyToEnemies()
{
	UWorld* world = GetWorld();
	if ( !world || Radius_ <= 0.f )
	{
		return;
	}

	AActor* instigator = Instigator_.Get();
	const IEntity* instigatorEntity = Cast<IEntity>( instigator );
	const ETeam instigatorTeam = instigatorEntity ? instigatorEntity->Team() : ETeam::Cat;

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params;
	params.AddIgnoredActor( this );
	if ( instigator )
	{
		params.AddIgnoredActor( instigator );
	}

	world->OverlapMultiByChannel(
		overlaps, GetActorLocation(), FQuat::Identity,
		ECC_Entity, FCollisionShape::MakeSphere( Radius_ ), params );

	const FVector center = GetActorLocation();
	const float radiusSq = Radius_ * Radius_;

	TSet<AActor*> seen;
	for ( const FOverlapResult& result : overlaps )
	{
		AActor* hitActor = result.GetActor();
		if ( !hitActor || seen.Contains( hitActor ) )
		{
			continue;
		}
		seen.Add( hitActor );

		const FVector enemyXY( hitActor->GetActorLocation().X, hitActor->GetActorLocation().Y, center.Z );
		if ( FVector::DistSquared( center, enemyXY ) > radiusSq )
		{
			continue;
		}

		IEntity* enemy = Cast<IEntity>( hitActor );
		if ( !enemy || !enemy->Stats().IsAlive() || enemy->Team() == instigatorTeam )
		{
			continue;
		}

		if ( DamagePerTick_ > 0 )
		{
			if ( instigator )
			{
				FDamageEvents::OnDamageDealt.Broadcast( instigator, hitActor, DamagePerTick_, true );
			}
			enemy->TakeDamage( DamagePerTick_, instigator );
		}

		if ( StatusPerTick_ )
		{
			if ( UStatusEffectTracker* tracker = UStatusEffectTracker::EnsureOn( hitActor ) )
			{
				tracker->ApplyStatus( StatusPerTick_, instigator );
			}
		}
	}
}
