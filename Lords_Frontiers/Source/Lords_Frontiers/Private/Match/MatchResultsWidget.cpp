#include "Lords_Frontiers/Public/Match/MatchResultsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Lords_Frontiers/Public/Match/LeaderboardSubsystem.h"
#include "Lords_Frontiers/Public/Match/MatchResultsLeaderboardRowWidget.h"
#include "Lords_Frontiers/Public/Match/MatchScoringConfig.h"
#include "Lords_Frontiers/Public/Match/MatchStatsTracker.h"

DEFINE_LOG_CATEGORY_STATIC( LogMatchResults, Log, All );

void UMatchResultsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ValidateState( /*bLogWarnings=*/true );
}

void UMatchResultsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateView();

	if ( RestartButton && !RestartButton->OnClicked.IsAlreadyBound( this, &UMatchResultsWidget::HandleRestartClicked ) )
	{
		RestartButton->OnClicked.AddDynamic( this, &UMatchResultsWidget::HandleRestartClicked );
	}
	if ( MainMenuButton && !MainMenuButton->OnClicked.IsAlreadyBound( this, &UMatchResultsWidget::HandleMainMenuClicked ) )
	{
		MainMenuButton->OnClicked.AddDynamic( this, &UMatchResultsWidget::HandleMainMenuClicked );
	}
}

void UMatchResultsWidget::HandleRestartClicked()
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
		{
			session->RestartGame();
		}
	}
	RemoveFromParent();
}

void UMatchResultsWidget::HandleMainMenuClicked()
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( const ULevelSubsystem* levels = gi->GetSubsystem<ULevelSubsystem>() )
		{
			levels->LoadMainMenu();
		}
	}
	RemoveFromParent();
}

UMatchStatsTracker* UMatchResultsWidget::GetTracker() const
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		return gi->GetSubsystem<UMatchStatsTracker>();
	}
	return nullptr;
}

ULeaderboardSubsystem* UMatchResultsWidget::GetLeaderboard() const
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		return gi->GetSubsystem<ULeaderboardSubsystem>();
	}
	return nullptr;
}

bool UMatchResultsWidget::IsTrackerReady() const
{
	return GetTracker() != nullptr;
}

bool UMatchResultsWidget::IsLeaderboardReady() const
{
	return GetLeaderboard() != nullptr;
}

bool UMatchResultsWidget::IsScoringConfigReady() const
{
	const UMatchStatsTracker* tracker = GetTracker();
	return tracker && tracker->GetConfig() != nullptr;
}

bool UMatchResultsWidget::ValidateState( bool bLogWarnings ) const
{
	bool bOk = true;

	if ( !GetGameInstance() )
	{
		if ( bLogWarnings )
		{
			UE_LOG( LogMatchResults, Warning, TEXT( "MatchResultsWidget: GameInstance отсутствует." ) );
		}
		bOk = false;
	}
	if ( !IsTrackerReady() )
	{
		if ( bLogWarnings )
		{
			UE_LOG( LogMatchResults, Warning, TEXT( "MatchResultsWidget: UMatchStatsTracker не найден." ) );
		}
		bOk = false;
	}
	if ( !IsLeaderboardReady() )
	{
		if ( bLogWarnings )
		{
			UE_LOG( LogMatchResults, Warning, TEXT( "MatchResultsWidget: ULeaderboardSubsystem не найден — таблица лидеров недоступна." ) );
		}
		bOk = false;
	}
	if ( !IsScoringConfigReady() )
	{
		if ( bLogWarnings )
		{
			UE_LOG( LogMatchResults, Warning, TEXT( "MatchResultsWidget: MatchScoringConfig не задан — счёт считается как 0." ) );
		}
		bOk = false;
	}
	return bOk;
}

void UMatchResultsWidget::ShowResults( EGameResult result, const FString& playerName )
{
	SessionResult = result;
	CachedPlayerName = playerName;

	ValidateState( /*bLogWarnings=*/true );
	Refresh();
	UpdateView();
	OnResultsReady( SessionResult, CachedBreakdown.Total, CachedStats );

	SetVisibility( ESlateVisibility::Visible );
}

void UMatchResultsWidget::Hide()
{
	SetVisibility( ESlateVisibility::Collapsed );
}

TArray<FLeaderboardEntry> UMatchResultsWidget::GetTopEntries( int32 count ) const
{
	if ( const ULeaderboardSubsystem* board = GetLeaderboard() )
	{
		return board->GetTopN( count );
	}
	return {};
}

TArray<FLeaderboardEntry> UMatchResultsWidget::GetLeaderboardView( int32 count ) const
{
	if ( const ULeaderboardSubsystem* board = GetLeaderboard() )
	{
		return board->GetCombinedTopN( count );
	}
	return {};
}

FLeaderboardEntry UMatchResultsWidget::GetPlayerBestEntry() const
{
	if ( const ULeaderboardSubsystem* board = GetLeaderboard() )
	{
		return board->GetPlayerBestEntry();
	}
	return {};
}

FLeaderboardEntry UMatchResultsWidget::GetPlayerCurrentEntry() const
{
	if ( const ULeaderboardSubsystem* board = GetLeaderboard() )
	{
		return board->GetPlayerCurrentEntry();
	}
	return {};
}

FText UMatchResultsWidget::FormatNumber( int64 value ) const
{
	const bool bNegative = value < 0;
	uint64 abs = bNegative ? static_cast<uint64>( -( value + 1 ) ) + 1 : static_cast<uint64>( value );

	FString digits;
	if ( abs == 0 )
	{
		digits = TEXT( "0" );
	}
	else
	{
		while ( abs > 0 )
		{
			digits.AppendChar( static_cast<TCHAR>( TEXT( '0' ) + ( abs % 10 ) ) );
			abs /= 10;
		}
	}

	FString out;
	for ( int32 i = 0; i < digits.Len(); ++i )
	{
		if ( i > 0 && i % 3 == 0 )
		{
			out.AppendChar( TEXT( ',' ) );
		}
		out.AppendChar( digits[i] );
	}
	out.ReverseString();
	if ( bNegative )
	{
		out.InsertAt( 0, TEXT( "-" ) );
	}
	return FText::FromString( out );
}

bool UMatchResultsWidget::HasMultiplePlayerRuns() const
{
	const ULeaderboardSubsystem* board = GetLeaderboard();
	if ( !board )
	{
		return false;
	}
	const FLeaderboardEntry best = board->GetPlayerBestEntry();
	const FLeaderboardEntry current = board->GetPlayerCurrentEntry();
	return board->HasPlayerEntries() && !( best == current );
}

bool UMatchResultsWidget::PushCurrentRunToLeaderboard( const FString& playerName )
{
	UMatchStatsTracker* tracker = GetTracker();
	if ( !tracker )
	{
		UE_LOG( LogMatchResults, Warning, TEXT( "PushCurrentRunToLeaderboard: tracker отсутствует." ) );
		return false;
	}
	if ( !GetLeaderboard() )
	{
		UE_LOG( LogMatchResults, Warning, TEXT( "PushCurrentRunToLeaderboard: leaderboard subsystem отсутствует." ) );
		return false;
	}

	const FString name = playerName.IsEmpty() ? CachedPlayerName : playerName;
	const bool bOk = tracker->FinalizeAndPush( name );
	Refresh();
	OnLeaderboardUpdated();
	return bOk;
}

bool UMatchResultsWidget::AddManualEntry( const FString& playerName, int64 score )
{
	ULeaderboardSubsystem* board = GetLeaderboard();
	if ( !board )
	{
		UE_LOG( LogMatchResults, Warning, TEXT( "AddManualEntry: leaderboard subsystem отсутствует." ) );
		return false;
	}

	int32 maxEntries = 10;
	if ( const UMatchStatsTracker* tracker = GetTracker() )
	{
		if ( const UMatchScoringConfig* cfg = tracker->GetConfig() )
		{
			maxEntries = cfg->LeaderboardSize;
		}
	}

	const bool bOk = board->AddManualEntry( playerName, score, maxEntries );
	Refresh();
	OnLeaderboardUpdated();
	return bOk;
}

void UMatchResultsWidget::UpdateView()
{
	ApplyTextsToBoundWidgets();
	RebuildLeaderboardRows();
}

static void SetIfBound( UTextBlock* widget, const FText& text )
{
	if ( widget )
	{
		widget->SetText( text );
	}
}

void UMatchResultsWidget::ApplyTextsToBoundWidgets()
{
	if ( ResultTitleText )
	{
		FText title;
		switch ( SessionResult )
		{
		case EGameResult::Win: title = FText::FromString( TEXT( "Победа" ) ); break;
		case EGameResult::Lose: title = FText::FromString( TEXT( "Поражение" ) ); break;
		case EGameResult::EndlessRun: title = FText::FromString( TEXT( "Бесконечный режим" ) ); break;
		case EGameResult::Abandoned: title = FText::FromString( TEXT( "Прервано" ) ); break;
		default: title = FText::GetEmpty(); break;
		}
		ResultTitleText->SetText( title );
	}

	SetIfBound( WavesSurvivedValue, FormatCount( CachedStats.WavesSurvived ) );
	SetIfBound( EnemiesKilledValue, FormatCount( FMath::Max( 0, CachedStats.EnemiesKilled - CachedStats.BossesKilled ) ) );
	SetIfBound( BossesKilledValue, FormatCount( CachedStats.BossesKilled ) );
	SetIfBound( DamageDealtValue, FormatScore( CachedStats.DamageDealt ) );

	SetIfBound( Tower_Speed_Value, FormatCount( CachedStats.TowersBuiltByType.FindRef( EDefensiveTowerType::Speed ) ) );
	SetIfBound( Tower_Sniper_Value, FormatCount( CachedStats.TowersBuiltByType.FindRef( EDefensiveTowerType::Sniper ) ) );
	SetIfBound( Tower_Magic_Value, FormatCount( CachedStats.TowersBuiltByType.FindRef( EDefensiveTowerType::Magic ) ) );
	SetIfBound( Tower_Mortar_Value, FormatCount( CachedStats.TowersBuiltByType.FindRef( EDefensiveTowerType::Mortar ) ) );
	SetIfBound( TowersTotalValue, FormatCount( CachedStats.TowersBuilt ) );

	SetIfBound( Resource_Mint_Value, FormatScore( CachedStats.ResourcesEarned.FindRef( EResourceType::Gold ) ) );
	SetIfBound( Resource_Fish_Value, FormatScore( CachedStats.ResourcesEarned.FindRef( EResourceType::Food ) ) );
	SetIfBound( Resource_Population_Value, FormatScore( CachedStats.ResourcesEarned.FindRef( EResourceType::Population ) ) );

	SetIfBound( TotalScoreValue, FormatScore( CachedBreakdown.Total ) );
	SetIfBound( BestScoreValue, FormatScore( CachedBestScore ) );
}

void UMatchResultsWidget::RebuildLeaderboardRows()
{
	if ( !LeaderboardContainer )
	{
		UE_LOG( LogTemp, Warning, TEXT( "MatchResults: LeaderboardContainer не привязан (BindWidgetOptional). Имя виджета должно быть точно 'LeaderboardContainer'." ) );
		return;
	}

	LeaderboardContainer->ClearChildren();

	const TArray<FLeaderboardEntry> view = GetLeaderboardView( LeaderboardRowCount );
	UE_LOG( LogTemp, Log, TEXT( "MatchResults: rebuild leaderboard, rows=%d, rowClass=%s" ), view.Num(), RowWidgetClass ? *RowWidgetClass->GetName() : TEXT( "<none>" ) );
	if ( const ULeaderboardSubsystem* board = GetLeaderboard() )
	{
		UE_LOG( LogTemp, Log, TEXT( "MatchResults: seeds=%d playerHas=%d best=%lld current=%lld" ),
			board->GetSeedCount(), board->HasPlayerEntries() ? 1 : 0,
			board->GetPlayerBestEntry().Score, board->GetPlayerCurrentEntry().Score );
	}
	for ( int32 i = 0; i < view.Num(); ++i )
	{
		const FLeaderboardEntry& entry = view[i];
		const FLinearColor color = entry.bIsCurrentPlayer ? PlayerHighlightColor : DefaultEntryColor;
		const int32 rank = i + 1;

		if ( RowWidgetClass )
		{
			UMatchResultsLeaderboardRowWidget* row = CreateWidget<UMatchResultsLeaderboardRowWidget>( this, RowWidgetClass );
			if ( row )
			{
				row->ApplyEntry( rank, entry, color, FormatScore( entry.Score ) );
				LeaderboardContainer->AddChild( row );
			}
			continue;
		}

		// Fallback, если класс строки не указан: голый HorizontalBox с тремя TextBlock.
		UHorizontalBox* row = NewObject<UHorizontalBox>( this );

		UTextBlock* rankText = NewObject<UTextBlock>( this );
		rankText->SetText( FText::FromString( FString::Printf( TEXT( "%d." ), rank ) ) );
		rankText->SetColorAndOpacity( FSlateColor( color ) );

		UTextBlock* nameText = NewObject<UTextBlock>( this );
		nameText->SetText( FText::FromString( entry.PlayerName ) );
		nameText->SetColorAndOpacity( FSlateColor( color ) );

		UTextBlock* scoreText = NewObject<UTextBlock>( this );
		scoreText->SetText( FormatScore( entry.Score ) );
		scoreText->SetColorAndOpacity( FSlateColor( color ) );

		row->AddChild( rankText );
		if ( UHorizontalBoxSlot* nameSlot = Cast<UHorizontalBoxSlot>( row->AddChild( nameText ) ) )
		{
			FSlateChildSize fill;
			fill.SizeRule = ESlateSizeRule::Fill;
			fill.Value = 1.f;
			nameSlot->SetSize( fill );
		}
		row->AddChild( scoreText );

		LeaderboardContainer->AddChild( row );
	}
}

UMatchResultsWidget* UMatchResultsWidget::ShowForLocalPlayer( UObject* worldContextObject, TSubclassOf<UMatchResultsWidget> widgetClass, EGameResult result )
{
	if ( !widgetClass )
	{
		return nullptr;
	}
	UWorld* world = GEngine ? GEngine->GetWorldFromContextObject( worldContextObject, EGetWorldErrorMode::LogAndReturnNull ) : nullptr;
	if ( !world )
	{
		return nullptr;
	}
	APlayerController* pc = UGameplayStatics::GetPlayerController( world, 0 );
	if ( !pc )
	{
		return nullptr;
	}
	UMatchResultsWidget* widget = CreateWidget<UMatchResultsWidget>( pc, widgetClass );
	if ( !widget )
	{
		return nullptr;
	}
	widget->AddToViewport( /*ZOrder=*/ 1000 );
	widget->ShowResults( result, FString() );
	return widget;
}

void UMatchResultsWidget::Refresh()
{
	bool bAnyData = false;

	if ( UMatchStatsTracker* tracker = GetTracker() )
	{
		CachedStats = tracker->GetStats();
		CachedBreakdown = tracker->ComputeBreakdown();
		bAnyData = true;
	}
	else
	{
		CachedStats = FMatchStats();
		CachedBreakdown = FScoreBreakdown();
	}

	if ( const ULeaderboardSubsystem* board = GetLeaderboard() )
	{
		CachedBestScore = board->GetBestScore();
		CachedPlayerRank = board->FindRankByScore( CachedBreakdown.Total );
		bAnyData = true;
	}
	else
	{
		CachedBestScore = 0;
		CachedPlayerRank = -1;
	}

	bDataReady = bAnyData;
}
