// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Debug/DebugPlayerController.h"
#include "Lords_Frontiers/Public/UI/Widgets/HealthBarWidget.h"
#include "Building/Construction/BuildManager.h"
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

	// В Project Settings -> Input -> Action Mappings добавь "ShowAllHP" на LeftAlt/RightAlt
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

	ChangedWidgetPrevVisibility_.Empty();

	UWorld* world = GetWorld();
	if ( !world )
		return;

	TArray<AActor*> found;
	UGameplayStatics::GetAllActorsOfClass( world, ABuilding::StaticClass(), found );

	for ( AActor* actor : found )
	{
		if ( !actor )
			continue;

		TArray<UWidgetComponent*> comps;
		actor->GetComponents<UWidgetComponent>( comps );

		for ( UWidgetComponent* wc : comps )
		{
			if ( !wc )
				continue;

			UUserWidget* inner = Cast<UUserWidget>( wc->GetUserWidgetObject() );
			const bool prevVisible = ( inner && inner->GetVisibility() == ESlateVisibility::Visible && !wc->bHiddenInGame );

			if ( !ChangedWidgetPrevVisibility_.Contains( wc ) )
			{
				ChangedWidgetPrevVisibility_.Add( wc, prevVisible );
			}

			wc->SetHiddenInGame( false );
			wc->SetVisibility( true );

			if ( inner )
			{
				inner->SetVisibility( ESlateVisibility::Visible );

				if ( UHealthBarWidget* hbw = Cast<UHealthBarWidget>( inner ) )
				{
					hbw->SuspendAutoHide( true );
					hbw->UpdateFromActor();
				}
			}
		}
	}
}

void ADebugPlayerController::OnShowAllHP_Released()
{
	AltHoldCount_ = FMath::Max( 0, AltHoldCount_ - 1 );

	if ( AltHoldCount_ != 0 )
		return;

	for ( auto it = ChangedWidgetPrevVisibility_.CreateIterator(); it; ++it )
	{
		UWidgetComponent* wc = it->Key.Get();
		const bool prevVisible = it->Value;

		if ( !wc )
			continue;

		UUserWidget* inner = Cast<UUserWidget>( wc->GetUserWidgetObject() );

		if ( prevVisible )
		{
			// restore condition before "ALT"
			wc->SetHiddenInGame( false );
			wc->SetVisibility( true );
			if ( inner )
			{
				inner->SetVisibility( ESlateVisibility::Visible );
				if ( UHealthBarWidget* hbw = Cast<UHealthBarWidget>( inner ) )
				{
					hbw->SuspendAutoHide( false );
					hbw->UpdateFromActor();
				}
			}
		}
		else
		{
			wc->SetHiddenInGame( false );
			wc->SetVisibility( true );

			if ( inner )
			{
				inner->SetVisibility( ESlateVisibility::Collapsed );
				if ( UHealthBarWidget* hbw = Cast<UHealthBarWidget>( inner ) )
				{
					hbw->SuspendAutoHide( false );
				}
			}
		}
	}

	ChangedWidgetPrevVisibility_.Empty();
}
