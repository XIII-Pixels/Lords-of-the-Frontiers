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

	inline double ReadStatValue( ABuilding* building, FName statName )
	{
		if ( !building )
		{
			return 0.0;
		}
		FNumericProperty* prop = FindNumericProperty( statName );
		if ( !prop )
		{
			return 0.0;
		}
		FEntityStats& stats = building->Stats();
		void* addr = prop->ContainerPtrToValuePtr<void>( &stats );

		if ( prop->IsFloatingPoint() )
		{
			return prop->GetFloatingPointPropertyValue( addr );
		}
		return static_cast<double>( prop->GetSignedIntPropertyValue( addr ) );
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

		if ( prop->IsFloatingPoint() )
		{
			const double current = prop->GetFloatingPointPropertyValue( addr );
			prop->SetFloatingPointPropertyValue( addr, current + delta );
			return;
		}

		if ( prop->IsInteger() )
		{
			const int64 current = prop->GetSignedIntPropertyValue( addr );
			const int64 rounded = static_cast<int64>( FMath::RoundToInt( delta ) );
			prop->SetIntPropertyValue( addr, current + rounded );
		}
	}
}
