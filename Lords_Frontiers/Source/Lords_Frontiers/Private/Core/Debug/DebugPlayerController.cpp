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

	UWorld* World = GetWorld();
	if ( !World )
		return;

	AHealthBarManager* Mgr =
	    Cast<AHealthBarManager>( UGameplayStatics::GetActorOfClass( World, AHealthBarManager::StaticClass() ) );

	if ( !Mgr )
	{
		UE_LOG( LogTemp, Warning, TEXT( "OnShowAllHP_Pressed: HealthBarManager not found in level" ) );
		return;
	}

	Mgr->ShowActiveWidgets();
}

void ADebugPlayerController::OnShowAllHP_Released()
{
	AltHoldCount_ = FMath::Max( 0, AltHoldCount_ - 1 );
	UE_LOG( LogTemp, Log, TEXT( "OnShowAllHP_Released (count=%d)" ), AltHoldCount_ );

	if ( AltHoldCount_ != 0 )
		return;

	UWorld* World = GetWorld();
	if ( !World )
		return;

	AHealthBarManager* Mgr =
	    Cast<AHealthBarManager>( UGameplayStatics::GetActorOfClass( World, AHealthBarManager::StaticClass() ) );

	if ( !Mgr )
	{
		UE_LOG( LogTemp, Warning, TEXT( "OnShowAllHP_Released: HealthBarManager not found in level" ) );
		return;
	}

	Mgr->HideActiveWidgets();
}