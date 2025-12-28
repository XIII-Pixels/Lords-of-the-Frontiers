// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Debug/DebugPlayerController.h"

#include "Building/Construction/BuildManager.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "UpgradeSystem/CardManager.h"

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

	InputComponent->BindAction("TestCards", IE_Pressed, this, &ADebugPlayerController::TestShowCards);
}

void ADebugPlayerController::TestShowCards()
{
	UE_LOG(LogTemp, Warning, TEXT("TestShowCards called!"));

	if (UCardManager* CardManager = GetWorld()->GetSubsystem<UCardManager>())
	{
		UE_LOG(LogTemp, Warning, TEXT("CardManager found, showing cards..."));
		CardManager->ShowCardSelection(3);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CardManager NOT found!"));
	}
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

	if ( BuildManager_->IsPlacing() && BuildManager_ )
	{
		BuildManager_->ConfirmPlacing();
		return;
	}

	USelectionManagerComponent* selection = GetSelectionManager();
	if ( !selection )
	{
		return;
	}

	// Трассировка под курсором по видимости
	FHitResult hit;
	const bool bHit = GetHitResultUnderCursorByChannel(
	    UEngineTypes::ConvertToTraceType( ECollisionChannel::ECC_Visibility ), false, hit
	);

	if ( !bHit )
	{
		// В пустоту кликнули – снимаем выделение
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "ClearSelection VOID" ) );
		selection->ClearSelection();
		return;
	}

	AActor* hitActor = hit.GetActor();
	if ( !IsValid( hitActor ) )
	{
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Other actor" ) );
		selection->ClearSelection();
		return;
	}

	// Пока на этом шаге: просто запоминаем одного выбранного актора.
	// Позже сюда добавим проверку ISelectable и вызов OnSelected/OnDeselected.
	GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "SelectSingle" ) );
	selection->SelectSingle( hitActor );
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