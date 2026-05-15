#include "Tutorial/TutorialSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Camera/StrategyCamera.h"
#include "Components/InputComponent.h"
#include "Components/MeshComponent.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Tutorial/TutorialConfig.h"

DEFINE_LOG_CATEGORY_STATIC( LogTutorial, Log, All );

UTutorialSubsystem* UTutorialSubsystem::Get( const UObject* worldContextObject )
{
	if ( !worldContextObject )
	{
		return nullptr;
	}
	const UWorld* world = worldContextObject->GetWorld();
	return world ? world->GetSubsystem<UTutorialSubsystem>() : nullptr;
}

bool UTutorialSubsystem::ShouldCreateSubsystem( UObject* outer ) const
{
	if ( !Super::ShouldCreateSubsystem( outer ) )
	{
		return false;
	}
	const UWorld* world = Cast<UWorld>( outer );
	return world && world->IsGameWorld();
}

void UTutorialSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );

	if ( UUserInterfaceSettings* settings = GetMutableDefault<UUserInterfaceSettings>() )
	{
		settings->UIScaleRule = EUIScalingRule::ShortestSide;
		settings->ApplicationScale = 1.0f;
		settings->UIScaleCurve.EditorCurveData.Reset();
		settings->UIScaleCurve.EditorCurveData.AddKey( 0.f, 1.f );
		settings->UIScaleCurve.EditorCurveData.AddKey( 100000.f, 1.f );
	}

	if ( FSlateApplication::IsInitialized() )
	{
		FSlateApplication::Get().SetApplicationScale( 1.f );
	}
}

void UTutorialSubsystem::Deinitialize()
{
	StopTutorial();
	Super::Deinitialize();
}

void UTutorialSubsystem::StartTutorial( UTutorialConfig* config )
{
	if ( bRunning_ )
	{
		UE_LOG( LogTutorial, Verbose, TEXT( "StartTutorial ignored: already running" ) );
		return;
	}
	if ( !config || config->Steps.Num() == 0 )
	{
		UE_LOG( LogTutorial, Warning, TEXT( "StartTutorial: config null or empty" ) );
		return;
	}

	Config_ = config;
	if ( !IsTutorialMap() )
	{
		UE_LOG( LogTutorial, Log, TEXT( "Skipping tutorial: map mismatch" ) );
		Config_ = nullptr;
		return;
	}

	CachedHighlightMaterial_ = config->HighlightOverlayMaterial.LoadSynchronous();

	if ( UGameLoopManager* gL = GetGameLoop() )
	{
		BoundGameLoop_ = gL;
		gL->OnPhaseChanged.AddDynamic( this, &UTutorialSubsystem::HandlePhaseChanged );
	}

	if ( AStrategyCamera* cam = GetStrategyCamera() )
	{
		cam->SetZoomToMax();
	}

	bRunning_ = true;
	UE_LOG( LogTutorial, Log, TEXT( "Tutorial started, %d steps" ), config->Steps.Num() );
	EnterStep( 0 );
}

void UTutorialSubsystem::StopTutorial()
{
	if ( !bRunning_ && !CurrentWidget_ && !SkipInputComponent_ )
	{
		return;
	}

	DisableSkipInputCapture();
	SkipInputComponent_ = nullptr;
	DestroyStepWidget();
	ClearHighlight();

	if ( AStrategyCamera* cam = GetStrategyCamera() )
	{
		cam->SetZoomDisabled( false );
		cam->SetCameraInputDisabled( false );
		cam->SetIgnoreZoomBoundsCurve( false );
	}

	if ( BoundGameLoop_.IsValid() )
	{
		BoundGameLoop_->OnPhaseChanged.RemoveDynamic( this, &UTutorialSubsystem::HandlePhaseChanged );
		BoundGameLoop_.Reset();
	}

	Config_ = nullptr;
	CachedHighlightMaterial_ = nullptr;
	CurrentStepIndex_ = INDEX_NONE;
	bRunning_ = false;
}

void UTutorialSubsystem::NotifyEndTurnPressed()
{
	if ( !bRunning_ || !Config_ || !Config_->Steps.IsValidIndex( CurrentStepIndex_ ) )
	{
		return;
	}
	if ( Config_->Steps[ CurrentStepIndex_ ].Advance == ETutorialAdvance::OnEndTurnPressed )
	{
		Advance();
	}
}

void UTutorialSubsystem::NotifyUpgradePicked()
{
	if ( !bRunning_ || !Config_ || !Config_->Steps.IsValidIndex( CurrentStepIndex_ ) )
	{
		return;
	}
	if ( Config_->Steps[ CurrentStepIndex_ ].Advance == ETutorialAdvance::OnUpgradePicked )
	{
		Advance();
	}
}

void UTutorialSubsystem::EnterStep( int32 index )
{
	DisableSkipInputCapture();
	DestroyStepWidget();
	ClearHighlight();

	CurrentStepIndex_ = index;

	if ( !Config_ || !Config_->Steps.IsValidIndex( index ) )
	{
		UE_LOG( LogTutorial, Log, TEXT( "Tutorial finished" ) );
		StopTutorial();
		return;
	}

	const FTutorialStep& step = Config_->Steps[ index ];
	UE_LOG(
	    LogTutorial,
	    Log,
	    TEXT( "EnterStep %d, widget=%s, advance=%d" ),
	    index,
	    *GetNameSafe( step.WidgetClass ),
	    static_cast<int32>( step.Advance )
	);

	SpawnStepWidget( step );
	ApplyHighlight( step.HighlightActorTags );

	const bool bWidgetActive = step.WidgetClass != nullptr;
	const bool bBlockCamera = !step.bAllowCameraMovement;

	if ( AStrategyCamera* cam = GetStrategyCamera() )
	{
		cam->SetCameraInputDisabled( bBlockCamera );
		cam->SetZoomDisabled( bBlockCamera );
		cam->SetIgnoreZoomBoundsCurve( !bBlockCamera );
	}

	if ( bWidgetActive )
	{
		EnsureCursorForWidget();
		EnableSkipInputCapture();
	}
}

void UTutorialSubsystem::Advance()
{
	LastAdvanceTime_ = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;
	EnterStep( CurrentStepIndex_ + 1 );
}

ETutorialAdvance UTutorialSubsystem::GetCurrentStepAdvance() const
{
	if ( !Config_ || !Config_->Steps.IsValidIndex( CurrentStepIndex_ ) )
	{
		return ETutorialAdvance::None;
	}
	return Config_->Steps[ CurrentStepIndex_ ].Advance;
}

void UTutorialSubsystem::OnBubbleClickAnywhere()
{
	if ( !Config_ || !Config_->Steps.IsValidIndex( CurrentStepIndex_ ) )
	{
		return;
	}

	if ( GetWorld() && LastAdvanceTime_ > 0.0
	     && GetWorld()->GetTimeSeconds() - LastAdvanceTime_ < 0.3 )
	{
		return;
	}

	if ( Config_->Steps[ CurrentStepIndex_ ].Advance == ETutorialAdvance::ClickAnywhere )
	{
		Advance();
	}
}

void UTutorialSubsystem::SpawnStepWidget( const FTutorialStep& step )
{
	if ( !step.WidgetClass )
	{
		return;
	}
	APlayerController* pc = GetPC();
	if ( !pc )
	{
		return;
	}
	CurrentWidget_ = CreateWidget<UUserWidget>( pc, step.WidgetClass );
	if ( CurrentWidget_ )
	{
		CurrentWidget_->AddToViewport( 1000 );
	}
}

void UTutorialSubsystem::DestroyStepWidget()
{
	if ( CurrentWidget_ )
	{
		CurrentWidget_->RemoveFromParent();
		CurrentWidget_ = nullptr;
	}
}

void UTutorialSubsystem::ApplyHighlight( const TArray<FName>& tags )
{
	if ( tags.Num() == 0 || !CachedHighlightMaterial_ )
	{
		return;
	}
	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	UMaterialInterface* material = CachedHighlightMaterial_;
	for ( TActorIterator<AActor> it( world ); it; ++it )
	{
		AActor* actor = *it;
		bool match = false;
		for ( const FName& tag : tags )
		{
			if ( actor->Tags.Contains( tag ) )
			{
				match = true;
				break;
			}
		}
		if ( !match )
		{
			continue;
		}

		actor->ForEachComponent<UMeshComponent>(
		    false, [ material ]( UMeshComponent* mesh ) { mesh->SetOverlayMaterial( material ); }
		);
		HighlightedActors_.Add( actor );
	}
	UE_LOG( LogTutorial, Log, TEXT( "Highlight applied to %d actors" ), HighlightedActors_.Num() );
}

void UTutorialSubsystem::ClearHighlight()
{
	for ( const TWeakObjectPtr<AActor>& weak : HighlightedActors_ )
	{
		AActor* actor = weak.Get();
		if ( !actor )
		{
			continue;
		}
		actor->ForEachComponent<UMeshComponent>(
		    false, []( UMeshComponent* mesh ) { mesh->SetOverlayMaterial( nullptr ); }
		);
	}
	HighlightedActors_.Reset();
}

void UTutorialSubsystem::EnableSkipInputCapture()
{
	APlayerController* pc = GetPC();
	if ( !pc )
	{
		UE_LOG( LogTutorial, Warning, TEXT( "EnableSkipInputCapture: no PlayerController" ) );
		return;
	}

	if ( !SkipInputComponent_ )
	{
		SkipInputComponent_ = NewObject<UInputComponent>( this, TEXT( "TutorialSkipInputComponent" ) );
		SkipInputComponent_->Priority = INT_MAX;
		SkipInputComponent_->bBlockInput = false;

		FInputKeyBinding kb( FInputChord( EKeys::LeftMouseButton, false, false, false, false ), IE_Pressed );
		kb.bConsumeInput = false;
		kb.KeyDelegate.GetDelegateForManualSet().BindUObject( this, &UTutorialSubsystem::HandleSkipInputPressed );
		SkipInputComponent_->KeyBindings.Add( kb );
	}

	pc->PushInputComponent( SkipInputComponent_ );
}

void UTutorialSubsystem::DisableSkipInputCapture()
{
	if ( !SkipInputComponent_ )
	{
		return;
	}
	if ( APlayerController* pc = GetPC() )
	{
		pc->PopInputComponent( SkipInputComponent_ );
	}
}

void UTutorialSubsystem::HandleSkipInputPressed()
{
	if ( !Config_ || !Config_->Steps.IsValidIndex( CurrentStepIndex_ ) )
	{
		return;
	}
	const ETutorialAdvance advance = Config_->Steps[ CurrentStepIndex_ ].Advance;
	UE_LOG( LogTutorial, Log, TEXT( "LMB consumed at step %d (advance=%d)" ),
	        CurrentStepIndex_, static_cast<int32>( advance ) );
	if ( advance == ETutorialAdvance::ClickAnywhere )
	{
		Advance();
	}
}

void UTutorialSubsystem::EnsureCursorForWidget()
{
	APlayerController* pc = GetPC();
	if ( !pc )
	{
		return;
	}
	pc->bShowMouseCursor = true;
	FInputModeGameAndUI inputMode;
	inputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
	pc->SetInputMode( inputMode );
}

void UTutorialSubsystem::HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	if ( !bRunning_ || !Config_ || !Config_->Steps.IsValidIndex( CurrentStepIndex_ ) )
	{
		return;
	}

	const ETutorialAdvance advance = Config_->Steps[ CurrentStepIndex_ ].Advance;

	if ( advance == ETutorialAdvance::OnWaveWon && oldPhase == EGameLoopPhase::Combat
	     && newPhase == EGameLoopPhase::Reward )
	{
		Advance();
		return;
	}

	if ( CurrentWidget_ && oldPhase == EGameLoopPhase::Reward && newPhase == EGameLoopPhase::Building )
	{
		ReassertInputCapture();
	}
}

void UTutorialSubsystem::ReassertInputCapture()
{
	APlayerController* pc = GetPC();
	if ( !pc )
	{
		return;
	}

	pc->bShowMouseCursor = true;
	FInputModeGameAndUI inputMode;
	inputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
	pc->SetInputMode( inputMode );

	if ( SkipInputComponent_ )
	{
		pc->PopInputComponent( SkipInputComponent_ );
		pc->PushInputComponent( SkipInputComponent_ );
	}
}

bool UTutorialSubsystem::IsTutorialMap() const
{
	if ( !Config_ || Config_->TutorialMapName.IsNone() )
	{
		return true;
	}
	const UWorld* world = GetWorld();
	if ( !world )
	{
		return false;
	}
	FString shortName = FPackageName::GetShortName( world->GetMapName() );
	shortName = UWorld::RemovePIEPrefix( shortName );
	const bool match = Config_->TutorialMapName == FName( *shortName );
	UE_LOG(
	    LogTutorial,
	    Log,
	    TEXT( "IsTutorialMap: current='%s' expected='%s' match=%s" ),
	    *shortName,
	    *Config_->TutorialMapName.ToString(),
	    match ? TEXT( "true" ) : TEXT( "false" )
	);
	return match;
}

APlayerController* UTutorialSubsystem::GetPC() const
{
	UWorld* world = GetWorld();
	return world ? world->GetFirstPlayerController() : nullptr;
}

AStrategyCamera* UTutorialSubsystem::GetStrategyCamera() const
{
	if ( APlayerController* pc = GetPC() )
	{
		return Cast<AStrategyCamera>( pc->GetPawn() );
	}
	return nullptr;
}

UGameLoopManager* UTutorialSubsystem::GetGameLoop() const
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		return core->GetGameLoop();
	}
	return nullptr;
}
