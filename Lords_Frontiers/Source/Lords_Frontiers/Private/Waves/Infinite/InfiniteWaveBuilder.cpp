#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteWaveBuilder.h"

#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"

#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Math/UnrealMathUtility.h"

namespace
{
	float ScalarFromBuff( const FEnemyBuff& buff, int32 axis )
	{
		switch ( axis )
		{
			case 0: return buff.HealthMultiplier;
			case 1: return buff.AttackRangeMultiplier;
			case 2: return buff.AttackDamageMultiplier;
			case 3: return buff.AttackCooldownMultiplier;
			case 4: return buff.MaxSpeedMultiplier;
		}
		return 1.f;
	}

	void SetScalar( FEnemyBuff& buff, int32 axis, float value )
	{
		switch ( axis )
		{
			case 0: buff.HealthMultiplier = value; break;
			case 1: buff.AttackRangeMultiplier = value; break;
			case 2: buff.AttackDamageMultiplier = value; break;
			case 3: buff.AttackCooldownMultiplier = value; break;
			case 4: buff.MaxSpeedMultiplier = value; break;
		}
	}

	FEnemyBuff CombineBuffsMultiplicative( const FEnemyBuff& a, const FEnemyBuff& b )
	{
		FEnemyBuff out;
		for ( int32 i = 0; i < 5; ++i )
		{
			SetScalar( out, i, ScalarFromBuff( a, i ) * ScalarFromBuff( b, i ) );
		}
		return out;
	}

	FEnemyBuff ClampBuffToCap( const FEnemyBuff& value, const FEnemyBuff& cap )
	{
		FEnemyBuff out;
		for ( int32 i = 0; i < 5; ++i )
		{
			const float v = ScalarFromBuff( value, i );
			const float c = ScalarFromBuff( cap, i );
			if ( c > 0.f )
			{
				SetScalar( out, i, FMath::Min( v, c ) );
			}
			else
			{
				SetScalar( out, i, v );
			}
		}
		return out;
	}

	FName SectorOfSpawnPoint( const AEnemyGroupSpawnPoint* point )
	{
		return point ? point->Sector : NAME_None;
	}
}

void UInfiniteWaveBuilder::Initialize( UInfiniteModeConfig* config, int32 sessionSeed )
{
	Config = config;
	SessionSeed = sessionSeed;
	ResetState();
}

void UInfiniteWaveBuilder::ResetState()
{
	CarryOverBudget = 0;
	LastApexWave = -10000;
	LastWaveSeenPreset_.Empty();
	LastWaveSeenSector_.Empty();
	LastThemeId = NAME_None;
	LastBudget = 0;
	LastScalingBuff = FEnemyBuff();
}

int32 UInfiniteWaveBuilder::ComputeBudget( int32 waveIndex ) const
{
	if ( !Config )
	{
		return 0;
	}

	const int32 stepsSinceStart = FMath::Max( 0, waveIndex - Config->StartWaveIndex );
	float budget = Config->StartingBudget + stepsSinceStart * Config->BudgetPerWave;

	if ( Config->BudgetCurve )
	{
		const float mult = Config->BudgetCurve->GetFloatValue( static_cast<float>( stepsSinceStart ) );
		budget *= FMath::Max( 0.0f, mult );
	}

	if ( IsSpikeWave( waveIndex ) )
	{
		budget *= FMath::Max( 1.0f, Config->SpikeBudgetMultiplier );
	}

	budget += CarryOverBudget;

	return FMath::Max( 0, FMath::RoundToInt( budget ) );
}

bool UInfiniteWaveBuilder::IsSpikeWave( int32 waveIndex ) const
{
	if ( !Config || Config->SpikeWaveEvery <= 0 )
	{
		return false;
	}

	const int32 stepsSinceStart = waveIndex - Config->StartWaveIndex;
	if ( stepsSinceStart <= 0 )
	{
		return false;
	}
	return ( stepsSinceStart % Config->SpikeWaveEvery ) == 0;
}

FEnemyBuff UInfiniteWaveBuilder::ComputeScalingBuff( int32 waveIndex ) const
{
	FEnemyBuff result;
	if ( !Config )
	{
		return result;
	}

	const FInfiniteScalingConfig& scaling = Config->Scaling;
	const int32 stepsSinceStart = FMath::Max( 0, waveIndex - Config->StartWaveIndex );
	const int32 stepDiv = FMath::Max( 1, scaling.StepEveryNWaves );
	const float rawSteps = static_cast<float>( stepsSinceStart / stepDiv );

	const float curvedSteps = scaling.ScalingCurve ? scaling.ScalingCurve->GetFloatValue( rawSteps ) : rawSteps;

	for ( int32 i = 0; i < 5; ++i )
	{
		const float base = ScalarFromBuff( scaling.BaseBuff, i );
		const float delta = ScalarFromBuff( scaling.PerWaveDelta, i );
		const float value = base + delta * curvedSteps;
		SetScalar( result, i, FMath::Max( 1.0f, value ) );
	}

	result = ClampBuffToCap( result, scaling.CapBuff );
	return result;
}

const FInfiniteTheme* UInfiniteWaveBuilder::RollTheme( FRandomStream& rng, int32 waveIndex, bool bSpike ) const
{
	if ( !Config || Config->Themes.Num() == 0 )
	{
		return nullptr;
	}

	float total = 0.f;
	for ( const FInfiniteTheme& theme : Config->Themes )
	{
		if ( waveIndex < theme.MinWaveIndex )
		{
			continue;
		}
		if ( theme.bSpikeOnly && !bSpike )
		{
			continue;
		}
		total += FMath::Max( 0.0f, theme.RollWeight );
	}

	if ( total <= 0.f )
	{
		return &Config->Themes[0];
	}

	float roll = rng.FRandRange( 0.f, total );
	for ( const FInfiniteTheme& theme : Config->Themes )
	{
		if ( waveIndex < theme.MinWaveIndex )
		{
			continue;
		}
		if ( theme.bSpikeOnly && !bSpike )
		{
			continue;
		}

		roll -= FMath::Max( 0.0f, theme.RollWeight );
		if ( roll <= 0.f )
		{
			return &theme;
		}
	}

	return &Config->Themes.Last();
}

float UInfiniteWaveBuilder::RecencyMultiplier( int32 lastSeenWave, int32 currentWave ) const
{
	if ( !Config || Config->RecentWaveMemory <= 0 )
	{
		return 1.f;
	}
	const int32 gap = currentWave - lastSeenWave;
	if ( gap > Config->RecentWaveMemory )
	{
		return 1.f;
	}
	return FMath::Clamp( Config->RecencyPenalty, 0.0f, 1.0f );
}

float UInfiniteWaveBuilder::PresetWeight(
    const UEnemyPresetData* preset, const FInfiniteTheme& theme, int32 waveIndex, int32 remainingBudget,
    const TMap<FName, int32>& usedThisWave, bool bCoreOnly
) const
{
	if ( !preset || !preset->EnemyClass )
	{
		return 0.f;
	}
	if ( preset->Cost > remainingBudget )
	{
		return 0.f;
	}
	if ( waveIndex < preset->MinWaveIndex )
	{
		return 0.f;
	}
	if ( preset->bBossOnly && !bCoreOnly )
	{
		return 0.f;
	}

	const int32 perWaveCap = preset->MaxPerWave > 0 ? preset->MaxPerWave : Config->GlobalMaxSamePresetPerWave;
	const int32 used = usedThisWave.FindRef( preset->GetFName() );
	if ( used >= perWaveCap )
	{
		return 0.f;
	}

	float w = FMath::Max( 0.0f, preset->BaseWeight );

	for ( const FThemeTagWeight& tw : theme.TagWeights )
	{
		if ( preset->HasTag( tw.Tag ) )
		{
			w *= FMath::Max( 0.0f, tw.Multiplier );
		}
	}

	if ( !bCoreOnly )
	{
		const float ratio = static_cast<float>( preset->Cost ) / FMath::Max( 1.f, static_cast<float>( remainingBudget ) );
		const float aff = 1.0f - FMath::Abs( ratio - 0.35f );
		w *= FMath::Clamp( aff, 0.4f, 1.0f );

		const float antiRepeat = FMath::Pow( 0.7f, static_cast<float>( used ) );
		w *= antiRepeat;

		if ( const int32* lastSeen = LastWaveSeenPreset_.Find( preset->GetFName() ) )
		{
			w *= RecencyMultiplier( *lastSeen, waveIndex );
		}
	}

	return w;
}

UEnemyPresetData* UInfiniteWaveBuilder::PickPreset(
    FRandomStream& rng, const FInfiniteTheme& theme, int32 waveIndex, int32 remainingBudget,
    const TMap<FName, int32>& usedThisWave, bool bCoreOnly, bool bOffThemeOnly
) const
{
	if ( !Config )
	{
		return nullptr;
	}

	float total = 0.f;
	TArray<float> weights;
	weights.Reserve( Config->Presets.Num() );

	for ( const TObjectPtr<UEnemyPresetData>& presetPtr : Config->Presets )
	{
		UEnemyPresetData* preset = presetPtr.Get();
		float w = PresetWeight( preset, theme, waveIndex, remainingBudget, usedThisWave, bCoreOnly );

		if ( bOffThemeOnly && w > 0.f )
		{
			bool bMatchesTheme = false;
			for ( const FThemeTagWeight& tw : theme.TagWeights )
			{
				if ( tw.Multiplier > 1.0f && preset->HasTag( tw.Tag ) )
				{
					bMatchesTheme = true;
					break;
				}
			}
			if ( bMatchesTheme )
			{
				w = 0.f;
			}
		}

		weights.Add( w );
		total += w;
	}

	if ( total <= 0.f )
	{
		return nullptr;
	}

	float roll = rng.FRandRange( 0.f, total );
	for ( int32 i = 0; i < weights.Num(); ++i )
	{
		roll -= weights[i];
		if ( roll <= 0.f )
		{
			return Config->Presets[i].Get();
		}
	}
	return Config->Presets.Last().Get();
}

TArray<FName> UInfiniteWaveBuilder::CollectAllSectors( UObject* worldContextObject ) const
{
	TArray<FName> sectors;
	if ( !worldContextObject )
	{
		return sectors;
	}

	UWorld* world = worldContextObject->GetWorld();
	if ( !world )
	{
		return sectors;
	}

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass( world, AEnemyGroupSpawnPoint::StaticClass(), actors );

	TSet<FName> seen;
	for ( AActor* a : actors )
	{
		const AEnemyGroupSpawnPoint* sp = Cast<AEnemyGroupSpawnPoint>( a );
		if ( sp && !sp->Sector.IsNone() )
		{
			if ( !seen.Contains( sp->Sector ) )
			{
				seen.Add( sp->Sector );
				sectors.Add( sp->Sector );
			}
		}
	}

	return sectors;
}

TArray<FName> UInfiniteWaveBuilder::PickActiveSectors(
    FRandomStream& rng, const FInfiniteTheme& theme, int32 waveIndex, const TArray<FName>& allSectors
) const
{
	TArray<FName> out;
	if ( allSectors.Num() == 0 )
	{
		return out;
	}

	const TArray<FName>& pool = allSectors;

	const int32 minN = FMath::Clamp( theme.MinActiveSectors, 1, pool.Num() );
	const int32 maxN = FMath::Clamp( theme.MaxActiveSectors, minN, pool.Num() );
	const int32 wantN = rng.RandRange( minN, maxN );

	TArray<float> weights;
	weights.Reserve( pool.Num() );
	float total = 0.f;
	for ( const FName& s : pool )
	{
		float w = 1.0f;
		if ( LastWaveSeenSector_.Contains( s ) )
		{
			w *= RecencyMultiplier( LastWaveSeenSector_.FindRef( s ), waveIndex );
		}
		weights.Add( w );
		total += w;
	}

	auto pickOne = [&]( const TSet<FName>& exclude ) -> int32
	{
		float runningTotal = 0.f;
		for ( int32 i = 0; i < pool.Num(); ++i )
		{
			if ( exclude.Contains( pool[i] ) )
			{
				continue;
			}
			runningTotal += weights[i];
		}
		if ( runningTotal <= 0.f )
		{
			return INDEX_NONE;
		}
		float r = rng.FRandRange( 0.f, runningTotal );
		for ( int32 i = 0; i < pool.Num(); ++i )
		{
			if ( exclude.Contains( pool[i] ) )
			{
				continue;
			}
			r -= weights[i];
			if ( r <= 0.f )
			{
				return i;
			}
		}
		return INDEX_NONE;
	};

	TSet<FName> taken;
	const int32 firstIdx = pickOne( taken );
	if ( firstIdx == INDEX_NONE )
	{
		return out;
	}
	out.Add( pool[firstIdx] );
	taken.Add( pool[firstIdx] );

	if ( wantN >= 2 && Config->SectorRing.Num() > 1 )
	{
		const int32 ringFirst = Config->SectorRing.IndexOfByKey( pool[firstIdx] );
		if ( ringFirst != INDEX_NONE && ( theme.bRequireOppositeSectors || theme.bRequireAdjacentSectors ) )
		{
			const int32 ringSize = Config->SectorRing.Num();
			TArray<FName> allowed;
			if ( theme.bRequireAdjacentSectors )
			{
				allowed.Add( Config->SectorRing[( ringFirst + 1 ) % ringSize] );
				allowed.Add( Config->SectorRing[( ringFirst - 1 + ringSize ) % ringSize] );
			}
			if ( theme.bRequireOppositeSectors )
			{
				const int32 half = ringSize / 2;
				allowed.Add( Config->SectorRing[( ringFirst + half ) % ringSize] );
				if ( ringSize >= 4 )
				{
					allowed.Add( Config->SectorRing[( ringFirst + half - 1 + ringSize ) % ringSize] );
					allowed.Add( Config->SectorRing[( ringFirst + half + 1 ) % ringSize] );
				}
			}

			TArray<FName> intersect;
			for ( const FName& s : allowed )
			{
				if ( pool.Contains( s ) && !taken.Contains( s ) )
				{
					intersect.Add( s );
				}
			}
			if ( intersect.Num() > 0 )
			{
				const FName picked = intersect[rng.RandRange( 0, intersect.Num() - 1 )];
				out.Add( picked );
				taken.Add( picked );
			}
		}
	}

	while ( out.Num() < wantN )
	{
		const int32 idx = pickOne( taken );
		if ( idx == INDEX_NONE )
		{
			break;
		}
		out.Add( pool[idx] );
		taken.Add( pool[idx] );
	}

	return out;
}

void UInfiniteWaveBuilder::AssignPortalsToPurchases(
    FRandomStream& rng, TArray<FInfinitePurchase>& purchases, const TArray<FName>& activeSectors,
    const TMap<FName, TArray<FName>>& sectorToPortalIds, const TArray<FName>& flyerFreePortals
)
{
	if ( activeSectors.Num() == 0 )
	{
		return;
	}

	FName bossSector = NAME_None;
	for ( const FInfinitePurchase& p : purchases )
	{
		if ( p.Preset && ( p.Preset->HasTag( EEnemyTag::Boss ) || p.Preset->HasTag( EEnemyTag::Apex ) ) )
		{
			bossSector = activeSectors[0];
			break;
		}
	}

	TMap<FName, FName> sectorByPresetId;
	int32 freeFlyersLeft = ( Config && Config->bAllowFlyerFreePortal ) ? 1 : 0;

	int32 rr = 0;

	for ( FInfinitePurchase& p : purchases )
	{
		if ( !p.Preset )
		{
			continue;
		}

		if ( p.Preset->HasTag( EEnemyTag::Boss ) || p.Preset->HasTag( EEnemyTag::Apex ) )
		{
			p.ChosenSector = bossSector;
		}
		else if ( p.Preset->HasTag( EEnemyTag::Flying ) && freeFlyersLeft > 0 && flyerFreePortals.Num() > 0
		          && rng.GetFraction() < 0.5f )
		{
			--freeFlyersLeft;
			p.ChosenPortalId = flyerFreePortals[rng.RandRange( 0, flyerFreePortals.Num() - 1 )];
			p.ChosenSector = NAME_None;
			continue;
		}
		else
		{
			FName preferred = sectorByPresetId.FindRef( p.Preset->GetFName() );
			const bool bIsSplash = p.Preset->HasTag( EEnemyTag::Splash );

			if ( !preferred.IsNone() && !bIsSplash )
			{
				p.ChosenSector = preferred;
			}
			else
			{
				FName chosen = NAME_None;
				if ( bIsSplash && !preferred.IsNone() && activeSectors.Num() > 1 )
				{
					for ( int32 attempt = 0; attempt < activeSectors.Num(); ++attempt )
					{
						const FName candidate = activeSectors[( rr + attempt ) % activeSectors.Num()];
						if ( candidate != preferred && candidate != bossSector )
						{
							chosen = candidate;
							break;
						}
					}
				}
				if ( chosen.IsNone() )
				{
					for ( int32 attempt = 0; attempt < activeSectors.Num(); ++attempt )
					{
						const FName candidate = activeSectors[( rr + attempt ) % activeSectors.Num()];
						if ( candidate != bossSector || activeSectors.Num() == 1 )
						{
							chosen = candidate;
							++rr;
							break;
						}
					}
				}
				p.ChosenSector = chosen.IsNone() ? activeSectors[0] : chosen;
			}

			sectorByPresetId.FindOrAdd( p.Preset->GetFName() ) = p.ChosenSector;
		}

		const TArray<FName>* portalsInSector = sectorToPortalIds.Find( p.ChosenSector );
		if ( portalsInSector && portalsInSector->Num() > 0 )
		{
			p.ChosenPortalId = ( *portalsInSector )[rng.RandRange( 0, portalsInSector->Num() - 1 )];
		}
	}
}

UWaveData* UInfiniteWaveBuilder::AssembleWaveData(
    const TArray<FInfinitePurchase>& purchases, const FInfiniteTheme& theme, const FEnemyBuff& scalingBuff, bool bSpike
) const
{
	UWaveData* waveData = NewObject<UWaveData>( const_cast<UInfiniteWaveBuilder*>( this ), NAME_None, RF_Transient );

	FEnemyBuff baseBuff = CombineBuffsMultiplicative( theme.ThemeBuff, scalingBuff );
	if ( bSpike && Config )
	{
		baseBuff = CombineBuffsMultiplicative( baseBuff, Config->SpikeExtraBuff );
	}
	if ( Config )
	{
		baseBuff = ClampBuffToCap( baseBuff, Config->Scaling.CapBuff );
	}

	for ( const FInfinitePurchase& p : purchases )
	{
		if ( !p.Preset || !p.Preset->EnemyClass || p.Times <= 0 )
		{
			continue;
		}
		if ( p.ChosenPortalId.IsNone() )
		{
			continue;
		}

		FEnemySpawnSettings& settings = waveData->EnemySpawnMap.FindOrAdd( p.Preset->EnemyClass );

		FPortalSpawnEntry entry;
		entry.SpawnPointId = p.ChosenPortalId;
		entry.Count = p.Preset->Count * p.Times;
		entry.StartDelay = p.Preset->StartDelay;
		entry.SpawnInterval = p.Preset->SpawnInterval;
		settings.Portals.Add( entry );

		settings.Buff = CombineBuffsMultiplicative( p.Preset->PresetBuff, baseBuff );
	}

	return waveData;
}

void UInfiniteWaveBuilder::RememberWave( int32 waveIndex, const TArray<FInfinitePurchase>& purchases, bool bApexSeen )
{
	for ( const FInfinitePurchase& p : purchases )
	{
		if ( p.Preset )
		{
			LastWaveSeenPreset_.FindOrAdd( p.Preset->GetFName() ) = waveIndex;
		}
		if ( !p.ChosenSector.IsNone() )
		{
			LastWaveSeenSector_.FindOrAdd( p.ChosenSector ) = waveIndex;
		}
	}
	if ( bApexSeen )
	{
		LastApexWave = waveIndex;
	}
}

UWaveData* UInfiniteWaveBuilder::BuildWave( int32 waveIndex, UObject* worldContextObject )
{
	if ( !Config || Config->Presets.Num() == 0 )
	{
		return nullptr;
	}

	const int32 perWaveSeed = SessionSeed ^ static_cast<int32>( ( static_cast<uint32>( waveIndex ) + 1u ) * 0x9E3779B9u );
	FRandomStream rng( perWaveSeed );

	const bool bSpike = IsSpikeWave( waveIndex );
	const int32 budgetStart = ComputeBudget( waveIndex );
	int32 budget = budgetStart;
	LastBudget = budget;

	const FEnemyBuff scalingBuff = ComputeScalingBuff( waveIndex );
	LastScalingBuff = scalingBuff;

	const FInfiniteTheme* themePtr = RollTheme( rng, waveIndex, bSpike );
	if ( !themePtr )
	{
		static const FInfiniteTheme fallbackTheme;
		themePtr = &fallbackTheme;
	}
	const FInfiniteTheme& theme = *themePtr;
	LastThemeId = theme.ThemeId;

	TArray<FInfinitePurchase> purchases;
	TMap<FName, int32> usedThisWave;
	int32 totalEnemies = 0;
	int32 ecoHarassers = 0;

	auto buyPreset = [&]( UEnemyPresetData* preset ) -> bool
	{
		if ( !preset )
		{
			return false;
		}
		if ( preset->Cost > budget )
		{
			return false;
		}
		if ( preset->HasTag( EEnemyTag::Apex ) )
		{
			if ( waveIndex - LastApexWave < Config->ApexCooldownWaves )
			{
				return false;
			}
		}
		if ( preset->HasTag( EEnemyTag::TargetsEconomy ) )
		{
			if ( ecoHarassers >= Config->MaxEcoHarassersPerWave )
			{
				return false;
			}
			++ecoHarassers;
		}

		const int32 wouldBe = totalEnemies + preset->Count;
		if ( wouldBe > Config->MaxEnemiesPerWave )
		{
			return false;
		}

		FInfinitePurchase purchase;
		purchase.Preset = preset;
		purchase.Times = 1;
		purchases.Add( purchase );

		budget -= preset->Cost;
		totalEnemies = wouldBe;
		usedThisWave.FindOrAdd( preset->GetFName() ) += 1;
		return true;
	};

	int32 coreBudget = FMath::RoundToInt( budgetStart * FMath::Clamp( theme.CoreBudgetFraction, 0.0f, 1.0f ) );
	UEnemyPresetData* coreAnchor = PickPreset( rng, theme, waveIndex, coreBudget, usedThisWave, true, false );
	if ( coreAnchor )
	{
		while ( coreAnchor && coreAnchor->Cost <= coreBudget && coreAnchor->Cost <= budget )
		{
			if ( !buyPreset( coreAnchor ) )
			{
				break;
			}
			coreBudget -= coreAnchor->Cost;
			const int32 cap = coreAnchor->MaxPerWave > 0 ? coreAnchor->MaxPerWave : Config->GlobalMaxSamePresetPerWave;
			if ( usedThisWave.FindRef( coreAnchor->GetFName() ) >= cap )
			{
				break;
			}
		}
	}

	for ( int32 safety = 0; safety < 64; ++safety )
	{
		UEnemyPresetData* next = PickPreset( rng, theme, waveIndex, budget, usedThisWave, false, false );
		if ( !next )
		{
			break;
		}
		if ( !buyPreset( next ) )
		{
			usedThisWave.FindOrAdd( next->GetFName() ) = Config->GlobalMaxSamePresetPerWave;
		}
	}

	if ( rng.GetFraction() < theme.SpiceChance )
	{
		UEnemyPresetData* spice = PickPreset( rng, theme, waveIndex, budget, usedThisWave, false, true );
		if ( spice )
		{
			buyPreset( spice );
		}
	}

	if ( Config->GuaranteeDefenseTargetingFromWave >= 0 && waveIndex >= Config->GuaranteeDefenseTargetingFromWave )
	{
		bool bHasDefenseTargeter = false;
		for ( const FInfinitePurchase& p : purchases )
		{
			if ( p.Preset
			     && ( p.Preset->HasTag( EEnemyTag::TargetsDefense ) || p.Preset->HasTag( EEnemyTag::Flying )
			          || p.Preset->HasTag( EEnemyTag::Ranged ) ) )
			{
				bHasDefenseTargeter = true;
				break;
			}
		}
		if ( !bHasDefenseTargeter )
		{
			UEnemyPresetData* fallback = nullptr;
			int32 cheapest = MAX_int32;
			for ( const TObjectPtr<UEnemyPresetData>& presetPtr : Config->Presets )
			{
				UEnemyPresetData* preset = presetPtr.Get();
				if ( !preset || waveIndex < preset->MinWaveIndex )
				{
					continue;
				}
				if ( preset->HasTag( EEnemyTag::TargetsDefense ) || preset->HasTag( EEnemyTag::Flying )
				     || preset->HasTag( EEnemyTag::Ranged ) )
				{
					if ( preset->Cost < cheapest )
					{
						cheapest = preset->Cost;
						fallback = preset;
					}
				}
			}
			if ( fallback )
			{
				budget = FMath::Max( budget, fallback->Cost );
				buyPreset( fallback );
			}
		}
	}

	CarryOverBudget = FMath::Max( 0, budget );

	UWorld* world = worldContextObject ? worldContextObject->GetWorld() : nullptr;

	TArray<FName> allSectors;
	TMap<FName, TArray<FName>> sectorToPortalIds;
	TMap<FName, TArray<FName>> sectorToBossPortalIds;
	TArray<FName> flyerFreePortals;

	if ( world )
	{
		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass( world, AEnemyGroupSpawnPoint::StaticClass(), actors );

		TSet<FName> seenSectors;
		for ( AActor* a : actors )
		{
			const AEnemyGroupSpawnPoint* sp = Cast<AEnemyGroupSpawnPoint>( a );
			if ( !sp || sp->SpawnPointId.IsNone() )
			{
				continue;
			}
			flyerFreePortals.Add( sp->SpawnPointId );

			const FName sectorName = sp->Sector.IsNone() ? FName( TEXT( "Default" ) ) : sp->Sector;
			sectorToPortalIds.FindOrAdd( sectorName ).Add( sp->SpawnPointId );
			if ( sp->bAllowBoss )
			{
				sectorToBossPortalIds.FindOrAdd( sectorName ).Add( sp->SpawnPointId );
			}
			if ( !seenSectors.Contains( sectorName ) )
			{
				seenSectors.Add( sectorName );
				allSectors.Add( sectorName );
			}
		}
	}

	TArray<FName> activeSectors = PickActiveSectors( rng, theme, waveIndex, allSectors );

	AssignPortalsToPurchases( rng, purchases, activeSectors, sectorToPortalIds, flyerFreePortals );

	for ( FInfinitePurchase& p : purchases )
	{
		if ( !p.Preset )
		{
			continue;
		}
		if ( p.Preset->HasTag( EEnemyTag::Boss ) || p.Preset->HasTag( EEnemyTag::Apex ) )
		{
			const TArray<FName>* bossPortals = sectorToBossPortalIds.Find( p.ChosenSector );
			if ( bossPortals && bossPortals->Num() > 0 )
			{
				p.ChosenPortalId = ( *bossPortals )[rng.RandRange( 0, bossPortals->Num() - 1 )];
			}
		}
	}

	bool bApexSeen = false;
	for ( const FInfinitePurchase& p : purchases )
	{
		if ( p.Preset && p.Preset->HasTag( EEnemyTag::Apex ) )
		{
			bApexSeen = true;
			break;
		}
	}
	RememberWave( waveIndex, purchases, bApexSeen );

	return AssembleWaveData( purchases, theme, scalingBuff, bSpike );
}
