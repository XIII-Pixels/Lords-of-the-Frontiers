// Fill out your copyright notice in the Description page of Project Settings.

#include "Building/Animation/ResourceCollectionAnimComponent.h"

#include "Building/Animation/ResourceAnimConfig.h"

#include "Camera/CameraComponent.h"
#include "Camera/StrategyCamera.h"
UResourceCollectionAnimComponent::UResourceCollectionAnimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UResourceCollectionAnimComponent::StartAnimation(
    UResourceAnimConfig* animConfig, bool bIsRuined, float waveDelay
)
{
	if ( bIsAnimating_ )
	{
		FinishAnimation();
	}

	AActor* owner = GetOwner();
	if ( !IsValid( owner ) || !animConfig )
	{
		return;
	}

	CachedAnimConfig_ = animConfig;
	bIsRuined_ = bIsRuined;
	WaveDelay_ = waveDelay;
	OriginalScale_ = owner->GetActorScale3D();
	Elapsed_ = 0.0f;
	bIsAnimating_ = true;

	SetComponentTickEnabled( true );
}

void UResourceCollectionAnimComponent::TickComponent(
    float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if ( !bIsAnimating_ || !CachedAnimConfig_ )
	{
		return;
	}

	AActor* owner = GetOwner();
	if ( !IsValid( owner ) )
	{
		FinishAnimation();
		return;
	}

	Elapsed_ += DeltaTime;

	const float effectiveElapsed = Elapsed_ - WaveDelay_;
	if ( effectiveElapsed < 0.0f )
	{
		return;
	}

	const FResourceCollectionAnimParams& scaleAnim = CachedAnimConfig_->BuildingScaleAnim;
	const float totalAlpha = FMath::Clamp( effectiveElapsed / scaleAnim.Duration, 0.0f, 1.0f );

	FVector currentScale = OriginalScale_;

	if ( !bIsRuined_ && !IsIconOnlyMode() )
	{
		if ( totalAlpha < scaleAnim.InflatePhaseEnd )
		{
			EvaluateInflatePhase( totalAlpha, currentScale );
		}
		else if ( totalAlpha < scaleAnim.HoldPhaseEnd )
		{
			currentScale = OriginalScale_ * scaleAnim.InflateScale;
		}
		else
		{
			EvaluateDeflatePhase( totalAlpha, currentScale );
		}
		owner->SetActorScale3D( currentScale );
	}
	if ( totalAlpha >= 1.0f )
	{
		FinishAnimation();
	}
}

bool UResourceCollectionAnimComponent::IsIconOnlyMode() const
{
	if ( !CachedAnimConfig_ )
	{
		return false;
	}
	return GetCameraZoom() >= CachedAnimConfig_->IconOnlyZoomThreshold;
}

float UResourceCollectionAnimComponent::GetCameraZoom() const
{
	if ( const UWorld* world = GetWorld() )
	{
		if ( const APlayerController* pc = world->GetFirstPlayerController() )
		{
			if ( const AStrategyCamera* cam = Cast<AStrategyCamera>( pc->GetPawn() ) )
			{
				if ( cam->Camera )
				{
					return cam->Camera->OrthoWidth;
				}
			}
		}
	}
	return 2048.0f;
}

void UResourceCollectionAnimComponent::EvaluateInflatePhase( float totalAlpha, FVector& outScale ) const
{
	const FResourceCollectionAnimParams& scaleAnim = CachedAnimConfig_->BuildingScaleAnim;
	const float phaseAlpha = totalAlpha / scaleAnim.InflatePhaseEnd;
	const float eased = FMath::InterpEaseOut( 0.0f, 1.0f, phaseAlpha, scaleAnim.EaseExponent );
	const float scale = FMath::Lerp( 1.0f, scaleAnim.InflateScale, eased );
	outScale = OriginalScale_ * scale;
}

void UResourceCollectionAnimComponent::EvaluateDeflatePhase( float totalAlpha, FVector& outScale ) const
{
	const FResourceCollectionAnimParams& scaleAnim = CachedAnimConfig_->BuildingScaleAnim;
	const float phaseAlpha = ( totalAlpha - scaleAnim.HoldPhaseEnd ) / ( 1.0f - scaleAnim.HoldPhaseEnd );
	const float eased = FMath::InterpEaseIn( 0.0f, 1.0f, phaseAlpha, scaleAnim.EaseExponent );
	const float scale = FMath::Lerp( scaleAnim.InflateScale, 1.0f, eased );
	outScale = OriginalScale_ * scale;
}

void UResourceCollectionAnimComponent::FinishAnimation()
{
	bIsAnimating_ = false;
	CachedAnimConfig_ = nullptr;
	SetComponentTickEnabled( false );

	AActor* owner = GetOwner();
	if ( IsValid( owner ) )
	{
		owner->SetActorScale3D( OriginalScale_ );
	}
}
