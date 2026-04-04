#include "Building/Construction/BuildPreviewActor.h"

#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"

ABuildPreviewActor::ABuildPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "PreviewMesh" ) );
	SetRootComponent( MeshComponent_ );

	// ������ �� ��������� � ��������� � �� ����������� ����.
	MeshComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	MeshComponent_->SetCastShadow( false );
	MeshComponent_->SetGenerateOverlapEvents( false );

	RangeDecalComponent_ = CreateDefaultSubobject<UDecalComponent>( TEXT( "RangeDecal" ) );
	RangeDecalComponent_->SetupAttachment( MeshComponent_ );
	RangeDecalComponent_->SetRelativeRotation( FRotator( -90.f, 0.f, 0.f ) );
	RangeDecalComponent_->SetVisibility( false );
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
		for ( int i = 0; i < MeshComponent_->GetMaterials().Num(); i++ )
		{
			MeshComponent_->SetMaterial( i, newMat );
		}
	}
}

void ABuildPreviewActor::SetPreviewMesh( UStaticMesh* mesh )
{
	// ������ ��������� StaticMesh � ���������� ������.
	if ( !MeshComponent_ )
	{
		return;
	}

	MeshComponent_->SetStaticMesh( mesh );
}
void ABuildPreviewActor::ShowAttackRange( const float Radius )
{
	if ( !RangeDecalComponent_ || !RangeIndicatorMaterial_ )
	{
		return;
	}

	RangeDecalComponent_->DecalSize = FVector( Radius, Radius, Radius );
	RangeDecalComponent_->SetDecalMaterial( RangeIndicatorMaterial_ );
	RangeDecalComponent_->SetVisibility( true );
}

void ABuildPreviewActor::HideAttackRange()
{
	if ( RangeDecalComponent_ )
	{
		RangeDecalComponent_->SetVisibility( false );
	}
}