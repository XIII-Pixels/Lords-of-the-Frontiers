#include "Lords_Frontiers/Public/Waves/EnemyGroup.h"
#include "Math/UnrealMathUtility.h" // FMath
#include "UObject/Class.h"

FEnemyGroup::FEnemyGroup()
	: EnemyClass(nullptr)
	, Count(0)
	, SpawnInterval(0.0f)
{
}

FEnemyGroup::FEnemyGroup ( TSubclassOf<AUnit> inClass, int32 inCount, float inSpawnInterval )
	: EnemyClass ( inClass )
	, Count ( FMath::Max ( 0, inCount ) ) // Amount of units >= 0
	, SpawnInterval ( inSpawnInterval )
{
}

bool FEnemyGroup::IsValid () const
{
	return ( EnemyClass != nullptr ) && ( Count > 0 );
}

bool FEnemyGroup::IsInstantSpawn () const
{
	return SpawnInterval <= 0.0f;
}

float FEnemyGroup::GetSpawnDelayForIndex ( int32 index ) const
{
	if ( index <= 0 || IsInstantSpawn () || Count <= 1 )
	{
		return 0.0f;
	}

	// if index outranges Count-1, this should handle it.
	return SpawnInterval * static_cast<float> ( index );
}

float FEnemyGroup::GetTotalSpawnDuration () const
{
	if ( IsInstantSpawn () || Count <= 1 )
	{
		return 0.0f;
	}

	return SpawnInterval * static_cast<float> ( FMath::Max ( 0, Count - 1 ) );
}


FString FEnemyGroup::GetEnemyClassName () const
{ 
	if ( EnemyClass ) 
	{
		// TSubclassOf -> UClass* ; safe to call GetName()
		return EnemyClass -> GetName ();
	}
	return FString ( TEXT ( "None" ) );
}