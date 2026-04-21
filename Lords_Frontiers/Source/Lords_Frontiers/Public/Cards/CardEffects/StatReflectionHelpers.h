#pragma once

#include "Building/Building.h"
#include "EntityStats.h"

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

namespace CardStatReflection
{
	inline FNumericProperty* FindNumericProperty( FName statName )
	{
		if ( statName.IsNone() )
		{
			return nullptr;
		}
		FProperty* prop = FEntityStats::StaticStruct()->FindPropertyByName( statName );
		return CastField<FNumericProperty>( prop );
	}

	inline void ApplyStatDelta( ABuilding* building, FName statName, float delta )
	{
		if ( !building || FMath::IsNearlyZero( delta ) )
		{
			return;
		}
		FNumericProperty* prop = FindNumericProperty( statName );
		if ( !prop )
		{
			return;
		}
		FEntityStats& stats = building->Stats();
		void* addr = prop->ContainerPtrToValuePtr<void>( &stats );
		const double current = prop->GetFloatingPointPropertyValue( addr );
		prop->SetFloatingPointPropertyValue( addr, current + delta );
	}
}
