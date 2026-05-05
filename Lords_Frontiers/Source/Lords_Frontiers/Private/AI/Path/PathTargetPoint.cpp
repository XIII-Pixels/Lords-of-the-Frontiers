// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/PathTargetPoint.h"

APathTargetPoint::APathTargetPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );

	Mesh_ = CreateDefaultSubobject<UStaticMeshComponent>( "Mesh" );
	Mesh_->SetupAttachment( RootComponent );
	Mesh_->SetMobility( EComponentMobility::Movable );
	Mesh_->CastShadow = false;
	Mesh_->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	Hide();
}

void APathTargetPoint::Show()
{
	Mesh_->SetVisibility( true );
}

void APathTargetPoint::Hide()
{
	Mesh_->SetVisibility( false );
}

void APathTargetPoint::IncreaseRefCount()
{
	RefCount_++;
}

void APathTargetPoint::DecreaseRefCount()
{
	RefCount_ = FMath::Max( RefCount_ - 1, 0 );
}
