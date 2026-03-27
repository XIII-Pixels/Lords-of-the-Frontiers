// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Debug/DebugPlayerController.h"
#include "Lords_Frontiers/Public/UI/Widgets/HealthBarWidget.h"
#include "Building/Construction/BuildManager.h"
#include "Lords_Frontiers/Public/UI/HealthBarManager.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "UI/Cards/CardSelectionHUDComponent.h"

#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

ADebugPlayerController::ADebugPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	SelectionManagerComponent_ = CreateDefaultSubobject<USelectionManagerComponent>( TEXT( "SelectionManager" ) );

	CardSelectionHUDComponent_ = CreateDefaultSubobject<UCardSelectionHUDComponent>( TEXT( "CardSelectionHUD" ) );
}

void ADebugPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	World->GetTimerManager().SetTimerForNextTick( FTimerDelegate::CreateLambda(
	    [this]()
	    {
		    AHealthBarManager* Found = this->CacheHealthBarManager();

		    if ( IsValid( Found ) )
		    {
			    UE_LOG(
			        LogTemp, Log,
			        TEXT( "ADebugPlayerController::BeginPlay: HealthBarManager cached successfully for %s -> %s (ptr=%p)" ),
			        *GetNameSafe( this ), *GetNameSafe( Found ), Found
			    );
		    }
		    else
		    {
			    UE_LOG(
			        LogTemp, Warning,
			        TEXT( "ADebugPlayerController::BeginPlay: HealthBarManager NOT found / NOT cached yet for %s" ),
			        *GetNameSafe( this )
			    );
		    }
	    }
	) );
}
USelectionManagerComponent* ADebugPlayerController::GetSelectionManager() const
{
	return SelectionManagerComponent_;
}

void ADebugPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// �� ������ ������ ���������, ��� InputComponent ����������.
	if ( !InputComponent )
	{
		return;
	}

	InputComponent->BindAction( "ShowAllHP", IE_Pressed, this, &ADebugPlayerController::OnShowAllHP_Pressed );
	InputComponent->BindAction( "ShowAllHP", IE_Released, this, &ADebugPlayerController::OnShowAllHP_Released );

	// ����������� ���������� ������� ���.
	// ����� ���� ������� Action Mapping, �� ��� ������� ���������� �������� ��
	// �����.
	InputComponent->BindKey( EKeys::LeftMouseButton, IE_Pressed, this, &ADebugPlayerController::HandleLeftClick );

	InputComponent->BindKey( EKeys::RightMouseButton, IE_Pressed, this, &ADebugPlayerController::HandleRightClick );

	InputComponent->BindKey( EKeys::Escape, IE_Pressed, this, &ADebugPlayerController::HandleEscape );
	UE_LOG( LogTemp, Log, TEXT( "ADebugPlayerController::BINDED" ));
}

void ADebugPlayerController::EnsureBuildManager()
{
	if ( BuildManager_ )
	{
		return;
	}

	if ( UWorld* world = GetWorld() )
	{
		AActor* found = UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() );

		if ( found )
		{
			BuildManager_ = Cast<ABuildManager>( found );
		}
	}
}

void ADebugPlayerController::HandleLeftClick()
{
	EnsureBuildManager();

	// First check pointer validity, then call methods on it
	if ( BuildManager_ && BuildManager_->IsPlacing() )
	{
		// Extra safety: ensure BuildManager_ still valid before call
		if ( IsValid( BuildManager_ ) )
		{
			BuildManager_->ConfirmPlacing();
		}
		else
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT(
			        "HandleLeftClick: BuildManager_ became invalid before "
			        "ConfirmPlacing"
			    )
			);
		}
		return;
	}

	USelectionManagerComponent* selection = GetSelectionManager();
	if ( !IsValid( selection ) )
	{
		UE_LOG( LogTemp, Verbose, TEXT( "HandleLeftClick: SelectionManager is NULL or invalid" ) );
		return;
	}

	// Trace under cursor
	FHitResult hit;
	const bool bHit = GetHitResultUnderCursorByChannel(
	    UEngineTypes::ConvertToTraceType( ECollisionChannel::ECC_Visibility ), false, hit
	);

	if ( !bHit )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "ClearSelection VOID" ) );
		}
		selection->ClearSelection();
		return;
	}

	AActor* hitActor = hit.GetActor();
	if ( !IsValid( hitActor ) )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Other actor" ) );
		}
		selection->ClearSelection();
		return;
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "SelectSingle" ) );
	}
	selection->SelectSingle( hitActor );
}

void ADebugPlayerController::HandleRightClick()
{
	// ������ �� ��, ��� � � ���: ������ �������������, ���� ��� �������.
	EnsureBuildManager();

	if ( BuildManager_ && BuildManager_->IsPlacing() )
	{
		BuildManager_->CancelPlacing();
	}
}

void ADebugPlayerController::HandleEscape()
{
	// ������ �� ��, ��� � � ���: ������ �������������, ���� ��� �������.
	EnsureBuildManager();

	if ( BuildManager_ && BuildManager_->IsPlacing() )
	{
		BuildManager_->CancelPlacing();
	}
}
void ADebugPlayerController::OnShowAllHP_Pressed()
{
	AltHoldCount_ = FMath::Max( 0, AltHoldCount_ ) + 1;
	UE_LOG( LogTemp, Log, TEXT( "OnShowAllHP_Pressed (count=%d)" ), AltHoldCount_ );

	if ( AltHoldCount_ != 1 )
		return;

	UWorld* world = GetWorld();
	if ( !world )
		return;

	AHealthBarManager* manager = CachedHealthBarManager_.Get();

	if ( !IsValid( manager ) )
	{
		manager = CacheHealthBarManager();
	}

	if ( IsValid( manager ) )
	{
		manager->ShowActiveWidgets();
	}
	else
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "ADebugPlayerController::OnShowAllHP_Pressed: HealthBarManager not available for actor %s — skipping UI update" ),
		    *GetNameSafe( this )
		);
	}
}

void ADebugPlayerController::OnShowAllHP_Released()
{
	AltHoldCount_ = FMath::Max( 0, AltHoldCount_ - 1 );
	UE_LOG( LogTemp, Log, TEXT( "OnShowAllHP_Released (count=%d)" ), AltHoldCount_ );

	if ( AltHoldCount_ != 0 )
		return;

		UWorld* world = GetWorld();
	if ( !world )
		return;

	AHealthBarManager* manager = CachedHealthBarManager_.Get();

	if ( !IsValid( manager ) )
	{
		manager = CacheHealthBarManager();
	}
	if ( IsValid( manager ) )
	{
		manager->HideActiveWidgets();
	}
	else
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "ADebugPlayerController::OnShowAllHP_Released: HealthBarManager not available for actor %s — skipping "
		          "UI update" ),
		    *GetNameSafe( this )
		);
	}
}

AHealthBarManager* ADebugPlayerController::CacheHealthBarManager()
{
	CachedHealthBarManager_.Reset();

	UWorld* World = GetWorld();
	if ( !World )
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "CacheHealthBarManager: GetWorld() == nullptr (Actor=%s)" ), *GetNameSafe( this )
		);
		return nullptr;
	}

	AActor* Found = UGameplayStatics::GetActorOfClass( World, AHealthBarManager::StaticClass() );

	UE_LOG(
	    LogTemp, Verbose, TEXT( "CacheHealthBarManager: GetActorOfClass -> ptr=%p name=%s class=%s" ), Found,
	    *GetNameSafe( Found ), Found ? *Found->GetClass()->GetName() : TEXT( "null" )
	);

	if ( !Found )
	{
		return nullptr;
	}

	if ( Found->HasAnyFlags( RF_ClassDefaultObject ) )
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "CacheHealthBarManager: GetActorOfClass returned CDO (Default__), ptr=%p class=%s name=%s" ), Found,
		    *GetNameSafe( Found->GetClass() ), *GetNameSafe( Found )
		);
		return nullptr;
	}

	if ( !Found->IsA( AHealthBarManager::StaticClass() ) )
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "CacheHealthBarManager: Found actor is NOT AHealthBarManager: ptr=%p class=%s name=%s" ), Found,
		    *GetNameSafe( Found->GetClass() ), *GetNameSafe( Found )
		);
		return nullptr;
	}

	AHealthBarManager* Mgr = Cast<AHealthBarManager>( Found );
	if ( !IsValid( Mgr ) )
	{
		UE_LOG( LogTemp, Error, TEXT( "CacheHealthBarManager: Cast succeeded but object is invalid (ptr=%p)" ), Found );
		return nullptr;
	}

	CachedHealthBarManager_ = Mgr;
	UE_LOG(
	    LogTemp, Log, TEXT( "CacheHealthBarManager: cached HealthBarManager = %s (ptr=%p)" ), *GetNameSafe( Mgr ), Mgr
	);
	return Mgr;
}