#include "Building/Construction/BuildPreviewActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ABuildPreviewActor::ABuildPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "PreviewMesh" ) );
	SetRootComponent( MeshComponent_ );

	MeshComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	MeshComponent_->SetCastShadow( false );
	MeshComponent_->SetGenerateOverlapEvents( false );
}

void ABuildPreviewActor::SetCanBuild( const bool bCanBuild )
{
	if ( !MeshComponent_ )
	{
		return;
	}

	UMaterialInterface* newMat = nullptr;

	if ( bCanBuild )
	{
		newMat = ValidMaterial_;
	}
	else
	{
		newMat = InvalidMaterial_;
	}

	if ( newMat )
	{
		MeshComponent_->SetMaterial( 0, newMat );
	}
}

void ABuildPreviewActor::SetPreviewMesh( UStaticMesh* mesh )
{
	if ( !MeshComponent_ )
	{
		return;
	}

	MeshComponent_->SetStaticMesh( mesh );
}
