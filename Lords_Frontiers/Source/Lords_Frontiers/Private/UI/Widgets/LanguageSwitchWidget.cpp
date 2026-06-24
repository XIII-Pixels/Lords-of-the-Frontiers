#include "UI/Widgets/LanguageSwitchWidget.h"

#include "Localization/GameLocalization.h"
#include "UI/Widgets/LanguageOptionWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Kismet/KismetInternationalizationLibrary.h"

ULanguageSwitchWidget::ULanguageSwitchWidget( const FObjectInitializer& objectInitializer )
    : Super( objectInitializer )
{
	FLanguageOption russian;
	russian.Culture = TEXT( "ru" );

	FLanguageOption english;
	english.Culture = TEXT( "en" );

	Languages = { russian, english };
}

void ULanguageSwitchWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	CurrentIndex_ = FindCurrentCultureIndex();
	UpdateCurrentDisplay();
}

void ULanguageSwitchWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RootButton )
	{
		RootButton->OnClicked.AddDynamic( this, &ULanguageSwitchWidget::HandleFlagClicked );
	}

	// Keep the flag and label in sync when the culture is changed from elsewhere.
	CultureChangedHandle_ =
	    FInternationalization::Get().OnCultureChanged().AddUObject( this, &ULanguageSwitchWidget::HandleCultureChanged );

	CurrentIndex_ = FindCurrentCultureIndex();
	UpdateCurrentDisplay();
	RebuildOptions();
	CloseList();
}

void ULanguageSwitchWidget::NativeDestruct()
{
	if ( RootButton )
	{
		RootButton->OnClicked.RemoveDynamic( this, &ULanguageSwitchWidget::HandleFlagClicked );
	}

	for ( ULanguageOptionWidget* option : OptionWidgets_ )
	{
		if ( option )
		{
			option->OnPicked.RemoveDynamic( this, &ULanguageSwitchWidget::HandleOptionPicked );
		}
	}
	OptionWidgets_.Reset();

	FInternationalization::Get().OnCultureChanged().Remove( CultureChangedHandle_ );

	Super::NativeDestruct();
}

void ULanguageSwitchWidget::HandleFlagClicked()
{
	if ( Languages.IsEmpty() )
	{
		return;
	}

	// With a bound dropdown the flag button folds/unfolds the list;
	// without one it keeps the legacy behavior of cycling through the languages.
	if ( !OptionWidgets_.IsEmpty() )
	{
		IsListOpen() ? CloseList() : OpenList();
	}
	else
	{
		ApplyOption( ( CurrentIndex_ + 1 ) % Languages.Num() );
	}
}

void ULanguageSwitchWidget::HandleOptionPicked( int32 optionIndex )
{
	ApplyOption( optionIndex );
	CloseList();
}

void ULanguageSwitchWidget::SetLanguage( const FString& culture )
{
	const int32 index = Languages.IndexOfByPredicate(
	    [&culture]( const FLanguageOption& option ) { return option.Culture == culture; }
	);
	if ( index != INDEX_NONE )
	{
		ApplyOption( index );
	}
}

FString ULanguageSwitchWidget::GetCurrentCulture() const
{
	return Languages.IsValidIndex( CurrentIndex_ ) ? Languages[CurrentIndex_].Culture : FString();
}

bool ULanguageSwitchWidget::IsListOpen() const
{
	const UWidget* listRoot = GetListRoot();
	return listRoot && listRoot->GetVisibility() != ESlateVisibility::Collapsed;
}

void ULanguageSwitchWidget::OpenList()
{
	if ( UWidget* listRoot = GetListRoot() )
	{
		listRoot->SetVisibility( ESlateVisibility::Visible );
	}
}

void ULanguageSwitchWidget::CloseList()
{
	if ( UWidget* listRoot = GetListRoot() )
	{
		listRoot->SetVisibility( ESlateVisibility::Collapsed );
	}
}

UWidget* ULanguageSwitchWidget::GetListRoot() const
{
	return OptionsRoot ? OptionsRoot.Get() : Cast<UWidget>( OptionsPanel.Get() );
}

void ULanguageSwitchWidget::RebuildOptions()
{
	OptionWidgets_.Reset();

	if ( !OptionsPanel )
	{
		return;
	}
	OptionsPanel->ClearChildren();

	if ( !OptionWidgetClass )
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "ULanguageSwitchWidget: OptionsPanel is bound but OptionWidgetClass is not set; "
		          "the flag button will cycle languages instead of opening the list" )
		);
		return;
	}

	for ( int32 index = 0; index < Languages.Num(); ++index )
	{
		// The current language already sits on the switch button — list only the alternatives.
		if ( index == CurrentIndex_ )
		{
			continue;
		}

		ULanguageOptionWidget* option = CreateWidget<ULanguageOptionWidget>( this, OptionWidgetClass );
		if ( !option )
		{
			continue;
		}

		option->Setup( Languages[index], ResolveDisplayName( Languages[index] ), index );
		option->OnPicked.AddDynamic( this, &ULanguageSwitchWidget::HandleOptionPicked );
		OptionsPanel->AddChild( option );
		OptionWidgets_.Add( option );
	}
}

void ULanguageSwitchWidget::ApplyFlagBrush( UImage* flagImage, const FLanguageOption& option )
{
	if ( !flagImage )
	{
		return;
	}

	if ( option.FlagBrush.GetResourceObject() )
	{
		flagImage->SetBrush( option.FlagBrush );
		// Undo a collapse from a previously missing brush.
		if ( flagImage->GetVisibility() == ESlateVisibility::Collapsed )
		{
			flagImage->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
		}
	}
	else
	{
		// Keeping the designer-default texture would show the wrong language's flag.
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "LanguageSwitchWidget: no FlagBrush set for culture '%s' — hiding the flag image. "
		          "Assign the flag textures in the Languages array on the WBP "
		          "(Doc/SettingsMenu_Setup.md, step 2.3)." ),
		    *option.Culture
		);
		flagImage->SetVisibility( ESlateVisibility::Collapsed );
	}
}

void ULanguageSwitchWidget::ApplyOption( int32 index )
{
	if ( !Languages.IsValidIndex( index ) )
	{
		return;
	}

	CurrentIndex_ = index;
	const FString& culture = Languages[index].Culture;

#if WITH_EDITOR
	if ( GIsEditor )
	{
		// Switching the real culture inside the editor would relocalize the editor itself.
		// The game localization preview (same as Edit -> Preview Game Language) translates
		// only game text instead.
		FTextLocalizationManager& textManager = FTextLocalizationManager::Get();
		if ( culture == LordsFrontiersLoc::GetNativeCulture() )
		{
			textManager.DisableGameLocalizationPreview();
		}
		else
		{
			// Enable alone does not store the culture: FindCurrentCultureIndex reads the
			// configured preview language back when the menu is reopened, so keep it in sync
			// or the flag resets to the native language while the preview stays on.
			textManager.ConfigureGameLocalizationPreviewLanguage( culture );
			textManager.EnableGameLocalizationPreview( culture );
			if ( !textManager.IsGameLocalizationPreviewEnabled() )
			{
				// EnableGameLocalizationPreview is a silent no-op when the engine does not know
				// the native game culture — i.e. no game translations were registered at startup.
				UE_LOG(
				    LogTemp, Warning,
				    TEXT( "LanguageSwitchWidget: game localization preview did not enable for '%s' — "
				          "game text stays on the native language. Check the GameLocalization lines "
				          "in the startup log: were the CSV translations registered?" ),
				    *culture
				);
			}
		}
	}
	else
#endif
	{
		// SaveToConfig writes [Internationalization] Culture into GameUserSettings.ini so the
		// engine restores the language on the next launch.
		UKismetInternationalizationLibrary::SetCurrentCulture( culture, true );
	}

	UpdateCurrentDisplay();
	RebuildOptions();
	OnLanguageChanged.Broadcast( culture );
}

void ULanguageSwitchWidget::HandleCultureChanged()
{
	CurrentIndex_ = FindCurrentCultureIndex();
	UpdateCurrentDisplay();
	RebuildOptions();
}

int32 ULanguageSwitchWidget::FindCurrentCultureIndex() const
{
	FInternationalization& i18n = FInternationalization::Get();
	FString currentCulture = i18n.GetCurrentCulture()->GetName();

#if WITH_EDITOR
	// In the editor the game language is driven by the localization preview, not by the
	// editor-wide culture.
	if ( GIsEditor )
	{
		FTextLocalizationManager& textManager = FTextLocalizationManager::Get();
		currentCulture = textManager.IsGameLocalizationPreviewEnabled()
		                   ? textManager.GetConfiguredGameLocalizationPreviewLanguage()
		                   : FString( LordsFrontiersLoc::GetNativeCulture() );
	}
#endif

	if ( currentCulture.IsEmpty() )
	{
		currentCulture = LordsFrontiersLoc::GetNativeCulture();
	}

	// The culture can be specific ("ru-RU"); match through its fallback chain.
	const TArray<FString> chain = i18n.GetPrioritizedCultureNames( currentCulture );
	const int32 index = Languages.IndexOfByPredicate(
	    [&chain]( const FLanguageOption& option ) { return chain.Contains( option.Culture ); }
	);
	return index != INDEX_NONE ? index : 0;
}

void ULanguageSwitchWidget::UpdateCurrentDisplay()
{
	if ( !Languages.IsValidIndex( CurrentIndex_ ) )
	{
		return;
	}

	const FLanguageOption& option = Languages[CurrentIndex_];

	if ( LanguageText )
	{
		LanguageText->SetText( ResolveDisplayName( option ) );
	}

	ApplyFlagBrush( CurrentFlagImage, option );
}

FText ULanguageSwitchWidget::ResolveDisplayName( const FLanguageOption& option ) const
{
	if ( !option.DisplayName.IsEmpty() )
	{
		return option.DisplayName;
	}

	if ( const FCulturePtr culture = FInternationalization::Get().GetCulture( option.Culture ) )
	{
		return FText::FromString( culture->GetNativeName() );
	}
	return FText::FromString( option.Culture );
}
