// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Debug/DebugGameMode.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "Core/Debug/DebugPlayerController.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Debug/DebugUIWidget.h"

ADebugGameMode::ADebugGameMode()
{
	// Указываем, что для этого GameMode должен использоваться наш кастомный контроллер.
	PlayerControllerClass = ADebugPlayerController::StaticClass();
}

void ADebugGameMode::BeginPlay()
{
	Super::BeginPlay();

	SetupStaticCamera();

	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( this, 0 ) )
	{
		if ( DebugHUDClass )
		{
			DebugHUD_ = CreateWidget<UUserWidget>( pc, DebugHUDClass );
			if ( DebugHUD_ )
			{
				DebugHUD_->AddToViewport();
			}
		}
	}
}

void ADebugGameMode::SetupStaticCamera() const
{
	// (Автор)
	// Настраиваем статичную камеру над центром грида.
	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	APlayerController* pc = world->GetFirstPlayerController();
	if ( !pc )
	{
		return;
	}

	ACameraActor* camera = nullptr;

	// 1) Пытаемся найти уже существующую камеру в уровне
	TArray<AActor*> foundCameras;
	UGameplayStatics::GetAllActorsOfClass( world, ACameraActor::StaticClass(), foundCameras );

	if ( foundCameras.Num() > 0 )
	{
		camera = Cast<ACameraActor>( foundCameras[0] );
	}

	// 2) Если камеры нет — создаём свою над центром грида
	if ( !camera )
	{
		FVector gridOrigin = FVector::ZeroVector;
		float cellSize = 100.0f;
		int32 width = 0;
		int32 height = 0;

		if ( AGridManager* grid =
		         Cast<AGridManager>( UGameplayStatics::GetActorOfClass( world, AGridManager::StaticClass() ) ) )
		{
			gridOrigin = grid->GetActorLocation();
			cellSize = grid->GetCellSize();
			width = grid->GetGridWidth();
			height = grid->GetGridHeight();
		}

		// Центр грида (по X/Y). Если менеджер не найден, всё останется по нулям.
		FVector gridCenter = gridOrigin;
		gridCenter.X += static_cast<float>( width ) * cellSize * 0.5f;
		gridCenter.Y += static_cast<float>( height ) * cellSize * 0.5f;

		// Позиция камеры: немного "позади и сверху" от центра
		const float distanceXY = 1500.0f;
		const float heightZ = 1500.0f;

		const FVector cameraLocation = gridCenter + FVector( -distanceXY, -distanceXY, heightZ );
		const FRotator cameraRotation = ( gridCenter - cameraLocation ).Rotation();

		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		camera = world->SpawnActor<ACameraActor>( cameraLocation, cameraRotation, spawnParams );
	}

	// 3) Переключаем вид на найденную/созданную камеру
	if ( camera )
	{
		pc->SetViewTarget( camera ); // ACameraActor* неявно приводится к AActor*
	}
}
