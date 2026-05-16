#include "Lords_Frontiers/Public/Match/MatchResultsLeaderboardRowWidget.h"

#include "Components/TextBlock.h"

void UMatchResultsLeaderboardRowWidget::ApplyEntry( int32 rank, const FLeaderboardEntry& entry, FLinearColor color, const FText& formattedScore )
{
	CachedRank = rank;
	CachedEntry = entry;

	const FSlateColor slate( color );

	if ( RankText )
	{
		RankText->SetText( FText::FromString( FString::Printf( TEXT( "%d." ), rank ) ) );
		RankText->SetColorAndOpacity( slate );
	}
	if ( PlayerNameText )
	{
		PlayerNameText->SetText( FText::FromString( entry.PlayerName ) );
		PlayerNameText->SetColorAndOpacity( slate );
	}
	if ( ScoreText )
	{
		ScoreText->SetText( formattedScore );
		ScoreText->SetColorAndOpacity( slate );
	}

	OnEntryApplied( rank, entry, color );
}
