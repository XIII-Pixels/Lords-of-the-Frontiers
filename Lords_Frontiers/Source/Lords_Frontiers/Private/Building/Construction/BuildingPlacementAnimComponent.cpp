// Fill out your copyright notice in the Description page of Project Settings.

#include "Building/Construction/BuildingPlacementAnimComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UBuildingPlacementAnimComponent::UBuildingPlacementAnimComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBuildingPlacementAnimComponent::StartAnimation( const FBuildPlacementAnimParams& inParams )
{
	Params_ = inParams;

	AActor* owner = GetOwner();
	if ( !IsValid( owner ) )
	{
		DestroyComponent();
		return;
	}

	OriginalLocation_ = owner->GetActorLocation();
	OriginalScale_ = owner->GetActorScale3D();

	Phase1End_ = Params_.Phase1EndParams;
	Phase2End_ = Params_.Phase2EndParams;

	CachedMesh_ = owner->FindComponentByClass<UStaticMeshComponent>();

	if ( CachedMesh_ )
	{
		const int32 numMaterials = CachedMesh_->GetNumMaterials();
		OriginalMaterials_.Reserve( numMaterials );
		DynamicMaterials_.Reserve( numMaterials );

		for ( int32 i = 0; i < numMaterials; ++i )
		{
			UMaterialInterface* originalMat = CachedMesh_->GetMaterial( i );
			OriginalMaterials_.Add( originalMat );

			UMaterialInstanceDynamic* dynMat = CachedMesh_->CreateDynamicMaterialInstance( i, originalMat );
			DynamicMaterials_.Add( dynMat );
		}
	}

	owner->SetActorScale3D( OriginalScale_ * Params_.ScaleMultiplier );

	for ( UMaterialInstanceDynamic* dynMat : DynamicMaterials_ )
	{
		if ( dynMat )
		{
			dynMat->SetVectorParameterValue( Params_.EmissiveParamName, Params_.FlashColor );
		}
	}

	Elapsed_ = 0.0f;
	bIsAnimating_ = true;
	SetComponentTickEnabled( true );
}

void UBuildingPlacementAnimComponent::TickComponent(
    float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction
)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	if ( !bIsAnimating_ )
	{
		return;
	}

	AActor* owner = GetOwner();
	if ( !owner )
	{
		FinishAnimation();
		return;
	}

	Elapsed_ += deltaTime;
	const float totalAlpha = FMath::Clamp( Elapsed_ / Params_.Duration, 0.0f, 1.0f );
	FVector currentScale = OriginalScale_;
	FVector currentLocation = OriginalLocation_;
	float flashIntensity = 0.0f;

	if ( totalAlpha < Phase1End_ )
	{
		EvaluateAppearPhase( totalAlpha, currentLocation, currentScale, flashIntensity );
	}
	else if ( totalAlpha < Phase2End_ )
	{
		EvaluateJumpPhase( totalAlpha, currentLocation );
	}
	else
	{
		EvaluateLandPhase( totalAlpha, currentScale, currentLocation );
	}

	owner->SetActorScale3D( currentScale );
	owner->SetActorLocation( currentLocation );
	ApplyFlashColor( flashIntensity );

	if ( totalAlpha >= 1.0f )
	{
		FinishAnimation();
	}
}

void UBuildingPlacementAnimComponent::EvaluateAppearPhase(
    const float totalAlpha, FVector& outLocation, FVector& outScale, float& outFlashintensity
) const
{
	const float phaseAlpha = totalAlpha / Phase1End_;
	const float eased = FMath::InterpEaseOut( 0.0f, 1.0f, phaseAlpha, Params_.EaseExponent );

	const float scale = FMath::Lerp( Params_.ScaleMultiplier, 1.0f, eased );
	outScale = OriginalScale_ * scale;
	outFlashintensity = 1.0f - eased;
}

void UBuildingPlacementAnimComponent::EvaluateJumpPhase( const float totalAlpha, FVector& outLocation ) const
{
	const float phaseAlpha = ( totalAlpha - Phase1End_ ) / (Phase2End_ - Phase1End_);

	const float eased = FMath::InterpEaseOut( 0.0f, 1.0f, phaseAlpha, Params_.EaseExponent );

	const float jumpOffset = Params_.JumpHeight * eased;
	outLocation = OriginalLocation_ + FVector( 0.0f, 0.0f, jumpOffset );
}

void UBuildingPlacementAnimComponent::EvaluateLandPhase(
    const float totalAlpha, FVector& outScale, FVector& outLocation
) const
{
	const float phaseAlpha = ( totalAlpha - Phase2End_ ) / ( 1.0f - Phase2End_ );
	const float eased = FMath::InterpEaseIn( 0.0f, 1.0f, phaseAlpha, Params_.EaseExponent );

	const float jumpOffset = Params_.JumpHeight * ( 1.0f - eased );
	outLocation = OriginalLocation_ + FVector( 0.0f, 0.0f, jumpOffset );

	const float squashStrength = FMath::Sin( phaseAlpha * PI );
	const float scaleZ = FMath::Lerp( 1.0f, 0.9f, squashStrength );
	const float scaleXY = FMath::Lerp( 1.0f, 1.05f, squashStrength );
	outScale = FVector( OriginalScale_.X * scaleXY, OriginalScale_.Y * scaleXY, OriginalScale_.Z * scaleZ );
}

void UBuildingPlacementAnimComponent::ApplyFlashColor( const float flashIntensity )
{
	if ( flashIntensity <= 0.0f )
	{
		return;
	}

	const FLinearColor currentFlash = FMath::Lerp( FLinearColor::Black, Params_.FlashColor, flashIntensity );
	for ( UMaterialInstanceDynamic* dynMat : DynamicMaterials_ )
	{
		if ( dynMat )
		{
			dynMat->SetVectorParameterValue( Params_.EmissiveParamName, currentFlash );
		}
	}
}

void UBuildingPlacementAnimComponent::FinishAnimation()
{
	bIsAnimating_ = false;
	SetComponentTickEnabled( false );

	AActor* owner = GetOwner();
	if ( IsValid( owner ) )
	{
		owner->SetActorLocation( OriginalLocation_ );
		owner->SetActorScale3D( OriginalScale_ );
	}

	if ( CachedMesh_ )
	{
		for ( int32 i = 0; i < OriginalMaterials_.Num(); ++i )
		{
			CachedMesh_->SetMaterial( i, OriginalMaterials_[i] );
		}
	}

	DestroyComponent();
}