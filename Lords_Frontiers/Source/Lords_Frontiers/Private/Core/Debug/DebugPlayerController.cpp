// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Debug/DebugPlayerController.h"

#include "Building/Construction/BuildManager.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

ADebugPlayerController::ADebugPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	SelectionManagerComponent = CreateDefaultSubobject<USelectionManagerComponent>( TEXT( "SelectionManager" ) );
}

USelectionManagerComponent* ADebugPlayerController::GetSelectionManager() const
{
	return SelectionManagerComponent;
}

void ADebugPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// На всякий случай проверяем, что InputComponent существует.
	if ( !InputComponent )
	{
		return;
	}

	// Привязываем обработчик нажатия ЛКМ.
	// Можно было сделать Action Mapping, но для отладки достаточно привязки по ключу.
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
    if (BuildManager_ && BuildManager_->IsPlacing())
    {
        // Extra safety: ensure BuildManager_ still valid before call
        if (IsValid(BuildManager_))
        {
            BuildManager_->ConfirmPlacing();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HandleLeftClick: BuildManager_ became invalid before ConfirmPlacing"));
        }
        return;
    }

    USelectionManagerComponent* selection = GetSelectionManager();
    if (!IsValid(selection))
    {
        UE_LOG(LogTemp, Verbose, TEXT("HandleLeftClick: SelectionManager is NULL or invalid"));
        return;
    }

    // Trace under cursor
    FHitResult hit;
    const bool bHit = GetHitResultUnderCursorByChannel(
        UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, hit
    );

    if (!bHit)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ClearSelection VOID"));
        }
        selection->ClearSelection();
        return;
    }

    AActor* hitActor = hit.GetActor();
    if (!IsValid(hitActor))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Other actor"));
        }
        selection->ClearSelection();
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("SelectSingle"));
    }
    selection->SelectSingle(hitActor);
}

void ADebugPlayerController::HandleRightClick()
{
	// Логика та же, что и у ПКМ: отмена строительства, если оно активно.
	EnsureBuildManager();

	if ( BuildManager_ && BuildManager_->IsPlacing() )
	{
		BuildManager_->CancelPlacing();
	}
}

void ADebugPlayerController::HandleEscape()
{
	// Логика та же, что и у ПКМ: отмена строительства, если оно активно.
	EnsureBuildManager();

	if ( BuildManager_ && BuildManager_->IsPlacing() )
	{
		BuildManager_->CancelPlacing();
	}
}