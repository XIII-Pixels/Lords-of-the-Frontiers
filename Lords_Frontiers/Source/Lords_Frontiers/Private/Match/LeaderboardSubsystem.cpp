#include "Lords_Frontiers/Public/Match/LeaderboardSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Lords_Frontiers/Public/Match/LeaderboardSaveData.h"

void ULeaderboardSubsystem::Load( const FString& slotName )
{
	LastSlotName = slotName;
	Entries.Reset();

	if ( !UGameplayStatics::DoesSaveGameExist( slotName, 0 ) )
	{
		return;
	}

	USaveGame* loaded = UGameplayStatics::LoadGameFromSlot( slotName, 0 );
	if ( ULeaderboardSaveData* data = Cast<ULeaderboardSaveData>( loaded ) )
	{
		Entries = data->Entries;
	}
}

void ULeaderboardSubsystem::Save( const FString& slotName )
{
	LastSlotName = slotName;

	ULeaderboardSaveData* data = Cast<ULeaderboardSaveData>(
	    UGameplayStatics::CreateSaveGameObject( ULeaderboardSaveData::StaticClass() )
	);
	if ( !data )
	{
		return;
	}
	data->Entries = Entries;
	UGameplayStatics::SaveGameToSlot( data, slotName, 0 );
}

bool ULeaderboardSubsystem::AddEntry( const FLeaderboardEntry& entry, int32 maxEntries )
{
	Entries.Add( entry );
	SortAndTrim( maxEntries );

	if ( !LastSlotName.IsEmpty() )
	{
		Save( LastSlotName );
	}

	return Entries.Contains( entry );
}

bool ULeaderboardSubsystem::AddManualEntry( const FString& playerName, int64 score, int32 maxEntries )
{
	FLeaderboardEntry entry;
	entry.PlayerName = playerName;
	entry.Score = score;
	entry.Timestamp = FDateTime::Now();
	entry.bManual = true;
	return AddEntry( entry, maxEntries );
}

TArray<FLeaderboardEntry> ULeaderboardSubsystem::GetTopN( int32 n ) const
{
	TArray<FLeaderboardEntry> out;
	const int32 count = FMath::Min( n, Entries.Num() );
	for ( int32 i = 0; i < count; ++i )
	{
		out.Add( Entries[i] );
	}
	return out;
}

int32 ULeaderboardSubsystem::FindRankByScore( int64 score ) const
{
	for ( int32 i = 0; i < Entries.Num(); ++i )
	{
		if ( Entries[i].Score <= score )
		{
			return i;
		}
	}
	return Entries.Num();
}

bool ULeaderboardSubsystem::RemoveAt( int32 index )
{
	if ( !Entries.IsValidIndex( index ) )
	{
		return false;
	}
	Entries.RemoveAt( index );

	if ( !LastSlotName.IsEmpty() )
	{
		Save( LastSlotName );
	}
	return true;
}

void ULeaderboardSubsystem::Clear()
{
	Entries.Reset();
	if ( !LastSlotName.IsEmpty() )
	{
		Save( LastSlotName );
	}
}

int64 ULeaderboardSubsystem::GetBestScore() const
{
	return Entries.Num() > 0 ? Entries[0].Score : 0;
}

void ULeaderboardSubsystem::LoadSeedsFromConfig( const ULeaderboardConfig* config )
{
	SeedEntries.Reset();
	if ( !config )
	{
		return;
	}
	for ( const FLeaderboardSeedEntry& seed : config->Entries )
	{
		FLeaderboardEntry entry;
		entry.PlayerName = seed.PlayerName;
		entry.Score = seed.Score;
		entry.Timestamp = FDateTime::MinValue();
		entry.bManual = false;
		entry.bIsCurrentPlayer = false;
		SeedEntries.Add( entry );
	}
}

bool ULeaderboardSubsystem::HasPlayerEntries() const
{
	for ( const FLeaderboardEntry& e : Entries )
	{
		if ( e.bIsCurrentPlayer )
		{
			return true;
		}
	}
	return false;
}

FLeaderboardEntry ULeaderboardSubsystem::GetPlayerBestEntry() const
{
	FLeaderboardEntry best;
	bool bFound = false;
	for ( const FLeaderboardEntry& e : Entries )
	{
		if ( !e.bIsCurrentPlayer )
		{
			continue;
		}
		if ( !bFound || e.Score > best.Score )
		{
			best = e;
			bFound = true;
		}
	}
	return best;
}

FLeaderboardEntry ULeaderboardSubsystem::GetPlayerCurrentEntry() const
{
	FLeaderboardEntry current;
	bool bFound = false;
	for ( const FLeaderboardEntry& e : Entries )
	{
		if ( !e.bIsCurrentPlayer )
		{
			continue;
		}
		if ( !bFound || e.Timestamp > current.Timestamp )
		{
			current = e;
			bFound = true;
		}
	}
	return current;
}

TArray<FLeaderboardEntry> ULeaderboardSubsystem::GetCombinedTopN( int32 n ) const
{
	TArray<FLeaderboardEntry> view = SeedEntries;

	view.Sort( []( const FLeaderboardEntry& a, const FLeaderboardEntry& b ) { return a.Score > b.Score; } );

	if ( n > 0 && view.Num() > n )
	{
		view.SetNum( n );
	}

	if ( HasPlayerEntries() )
	{
		const FLeaderboardEntry best = GetPlayerBestEntry();
		const FLeaderboardEntry current = GetPlayerCurrentEntry();

		auto containsEntry = [&]( const FLeaderboardEntry& e )
		{
			for ( const FLeaderboardEntry& x : view )
			{
				if ( x == e )
				{
					return true;
				}
			}
			return false;
		};

		if ( !containsEntry( best ) )
		{
			view.Add( best );
		}
		if ( !( best == current ) && !containsEntry( current ) )
		{
			view.Add( current );
		}

		view.Sort( []( const FLeaderboardEntry& a, const FLeaderboardEntry& b ) { return a.Score > b.Score; } );
	}
	return view;
}

void ULeaderboardSubsystem::SortAndTrim( int32 maxEntries )
{
	Entries.Sort( []( const FLeaderboardEntry& a, const FLeaderboardEntry& b ) { return a.Score > b.Score; } );

	if ( maxEntries > 0 && Entries.Num() > maxEntries )
	{
		Entries.SetNum( maxEntries );
	}
}
