// WallTypes.cpp

#include "Building/WallTypes.h"

#include "Engine/StaticMesh.h"

UStaticMesh* FWallMeshSet::GetMeshForDirection( const EWallDirection direction ) const
{
	switch ( direction )
	{
	case EWallDirection::Vertical:
		return VerticalMesh;
	case EWallDirection::Horizontal:
		return HorizontalMesh;
	default:
		return nullptr;
	}
}
