// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Debug/DebugPlayerController.h"

#include "Building/Construction/BuildManager.h"
#include "Cards/CardSubsystem.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "UI/Cards/CardSelectionHUDComponent.h"
#include "Selectable.h"
#include "Utilities/TraceChannelMappings.h"

#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
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

void ADebugPlayerController::OnPossess( APawn* pawn )
{
	Super::OnPossess( pawn );

	if ( !pawn )
	{
		return;
	}

	auto logListenerNotFound = []()
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "ADebugPlayerController: component of possessed pawn named AudioListener is not found" )
		);
	};

	USceneComponent* listenerComp = Cast<USceneComponent>( pawn->GetDefaultSubobjectByName( TEXT( "AudioListener" ) ) );
	if ( !listenerComp )
	{
		logListenerNotFound();
		return;
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(
	    [this, listenerComp, logListenerNotFound]()
	    {
		    if ( !listenerComp )
		    {
			    logListenerNotFound();
			    return;
		    }

		    // Set listener to center of actor (which should be on ground)
		    SetAudioListenerOverride( listenerComp, FVector::ZeroVector, FRotator::ZeroRotator );

		    FVector location;
		    FVector front;
		    FVector right;
		    GetAudioListenerPosition( location, front, right );

		    UE_LOG(
		        LogTemp, Log, TEXT( "ADebugPlayerController: audio listener position overriden: %s" ),
		        *location.ToString()
		    );
		    UE_LOG( LogTemp, Log, TEXT( "ADebugPlayerController: Front: %s Right: %s" ), *front.ToString(), *right.ToString() );
	    }
	);
}

void ADebugPlayerController::Tick( float deltaSeconds )
{
	Super::Tick( deltaSeconds );
}

void ADebugPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// �� ������ ������ ���������, ��� InputComponent ����������.
	if ( !InputComponent )
	{
		return;
	}

	// ����������� ���������� ������� ���.
	// ����� ���� ������� Action Mapping, �� ��� ������� ���������� �������� ��
	// �����.
	InputComponent->BindKey( EKeys::LeftMouseButton, IE_Pressed, this, &ADebugPlayerController::HandleLeftClick );

	InputComponent->BindKey( EKeys::RightMouseButton, IE_Pressed, this, &ADebugPlayerController::HandleRightClick );

	InputComponent->BindKey( EKeys::Escape, IE_Pressed, this, &ADebugPlayerController::HandleEscape );
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
			    TEXT( "HandleLeftClick: BuildManager_ became invalid before "
			          "ConfirmPlacing" )
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

	// Primary cursor trace by Visibility — handles buildings whose root box
	// blocks the Visibility channel (houses, walls, etc.).
	FHitResult hit;
	bool bHit = GetHitResultUnderCursorByChannel(
	    UEngineTypes::ConvertToTraceType( ECollisionChannel::ECC_Visibility ), false, hit
	);

	AActor* hitActor = bHit ? hit.GetActor() : nullptr;
	const bool bHitSelectable = IsValid( hitActor ) && hitActor->Implements<USelectable>();

	// Fallback: object-type trace against ECC_Entity, for selectables whose
	// BP leaves the Visibility channel unblocked (defensive towers).
	if ( !bHitSelectable )
	{
		FVector worldOrigin = FVector::ZeroVector;
		FVector worldDir = FVector::ZeroVector;
		if ( DeprojectMousePositionToWorld( worldOrigin, worldDir ) )
		{
			if ( UWorld* world = GetWorld() )
			{
				const FVector traceEnd = worldOrigin + worldDir * 100000.0f;
				FCollisionObjectQueryParams objParams;
				objParams.AddObjectTypesToQuery( ECC_Entity );
				FCollisionQueryParams queryParams( SCENE_QUERY_STAT( SelectionEntityTrace ), false );

				FHitResult entityHit;
				if ( world->LineTraceSingleByObjectType( entityHit, worldOrigin, traceEnd, objParams, queryParams ) )
				{
					if ( AActor* entityActor = entityHit.GetActor() )
					{
						if ( entityActor->Implements<USelectable>() )
						{
							hitActor = entityActor;
							bHit = true;
						}
					}
				}
			}
		}
	}

	if ( !bHit || !IsValid( hitActor ) )
	{
		selection->ClearSelection();
		return;
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
		return;
	}

	// When not placing, RMB cancels the current object selection.
	if ( USelectionManagerComponent* selection = GetSelectionManager() )
	{
		selection->ClearSelection();
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

void ADebugPlayerController::Card_ToggleDebug()
{
	if ( UCardSubsystem* cards = UCardSubsystem::Get( this ) )
	{
		cards->ToggleDebugShowAll();
	}
}

void ADebugPlayerController::Card_Apply( FName cardID )
{
	if ( UCardSubsystem* cards = UCardSubsystem::Get( this ) )
	{
		cards->DebugApplyCardByID( cardID );
	}
}

void ADebugPlayerController::Card_Unlock( FName cardID )
{
	if ( UCardSubsystem* cards = UCardSubsystem::Get( this ) )
	{
		cards->UnlockCardByID( cardID );
	}
}

void ADebugPlayerController::Card_Lock( FName cardID )
{
	if ( UCardSubsystem* cards = UCardSubsystem::Get( this ) )
	{
		cards->LockCardByID( cardID );
	}
}

void ADebugPlayerController::Card_ResetUnlocks()
{
	if ( UCardSubsystem* cards = UCardSubsystem::Get( this ) )
	{
		cards->ResetUnlocksToDefaults();
	}
}
