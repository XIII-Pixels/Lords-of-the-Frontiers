// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Debug/DebugUIWidget.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"
#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Components/Button.h"
#include "Core/Debug/DebugPlayerController.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "Engine/Engine.h"
#include "Grid/GridVisualizer.h"
#include "Kismet/GameplayStatics.h"

void UDebugUIWidget::OnButton1Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 1 Clicked" ) );
	}

	// Если ещё не знаем визуализатор — попробуем найти его сейчас.
	if ( !GridVisualizer )
	{
		if ( UWorld* world = GetWorld() )
		{
			GridVisualizer =
			    Cast<AGridVisualizer>( UGameplayStatics::GetActorOfClass( world, AGridVisualizer::StaticClass() ) );
		}

		if ( !GridVisualizer )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "GridVisualizer not found" ) );
			}
			return;
		}
	}

	FLinearColor CurrentColor = Button1->GetBackgroundColor();
	if ( CurrentColor.Equals( FLinearColor::Red, 0.01f ) )
	{
		Button1->SetBackgroundColor( FLinearColor::Green );
	}
	else
	{
		Button1->SetBackgroundColor( FLinearColor::Red );
	}

	// Переключаем видимость доп. кнопок.
	bExtraButtonsVisible = !bExtraButtonsVisible;
	UpdateExtraButtonsVisibility();

	// Переключаем видимость сетки.
	if ( GridVisualizer->IsGridVisible() )
	{
		GridVisualizer->HideGrid();
	}
	else
	{
		GridVisualizer->ShowGrid();
	}
}

void UDebugUIWidget::OnButton2Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 2 Clicked (Build)" ) );
	}

	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "BuildManager not found" ) );
		}
		return;
	}

	if ( !Button2BuildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Button2BuildingClass is not set" ) );
		}
		return;
	}

	BuildManager->StartPlacingBuilding( Button2BuildingClass );
}

void UDebugUIWidget::OnButton3Clicked()
{
	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "BuildManager not found" ) );
		}
		return;
	}

	if ( !Button3BuildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Button3BuildingClass is not set" ) );
		}
		return;
	}

	BuildManager->StartPlacingBuilding( Button3BuildingClass );
}

void UDebugUIWidget::OnButton4Clicked()
{
	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager =
			    Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "BuildManager not found" ) );
		}
		return;
	}

	if ( !Button4BuildingClass )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "Button4BuildingClass is not set" ) );
		}
		return;
	}

	BuildManager->StartPlacingBuilding( Button4BuildingClass );
}

void UDebugUIWidget::OnButton7Clicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button 7 Clicked (Relocate Building)" ) );
	}

	// 1) Убедимся, что у нас есть BuildManager
	if ( !BuildManager )
	{
		if ( UWorld* world = GetWorld() )
		{
			BuildManager = Cast<ABuildManager>( UGameplayStatics::GetActorOfClass( world, ABuildManager::StaticClass() ) );
		}
	}

	if ( !BuildManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: BuildManager not found" )
			);
		}
		return;
	}

	if ( !SelectionManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: SelectionManager not found" )
			);
		}
		return;
	}

	// 3) Берём выделенное здание
	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	if ( !selectedBuilding )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Red, TEXT( "OnButton7Clicked: no building selected" ) );
		}
		return;
	}

	// 4) Запускаем перенос через BuildManager
	BuildManager->StartRelocatingBuilding ( selectedBuilding );
}

void UDebugUIWidget::StartOrAdvanceWave(AWaveManager* WaveManager)
{
	if (!WaveManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugUI: StartOrAdvanceWave called with null WaveManager"));
		return;
	}

	// If the first wave wasn't requested yet -> start the first wave.
	if (WaveManager->IsFirstWaveRequested() == false)
	{
		WaveManager->StartWaveAtIndex(0);
		UE_LOG(LogTemp, Log, TEXT("DebugUI: StartOrAdvanceWave -> StartWaveAtIndex(0)"));
	}
	else
	{
		WaveManager->AdvanceToNextWave();
		UE_LOG(LogTemp, Log, TEXT("DebugUI: StartOrAdvanceWave -> AdvanceToNextWave()"));
	}
}

void UDebugUIWidget::OnButtonEnemyWaveClicked()
{
	// Debug notify
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ButtonEnemyWave clicked - spawn/wave trigger DEBUG"));
	}

	UE_LOG(LogTemp, Log, TEXT("DebugUI: OnButtonEnemyWaveClicked - WaveManagerPtr.IsValid() = %d"),
		WaveManagerPtr.IsValid() ? 1 : 0);

	// Ensure we have a cached WaveManager (try to find if not cached)
	if (!WaveManagerPtr.IsValid())
	{
		FindAndCacheWaveManager();
	}

	if (WaveManagerPtr.IsValid())
	{
		AWaveManager* waveManager = WaveManagerPtr.Get();
		UE_LOG(LogTemp, Log, TEXT("DebugUI: WaveManagerPtr.Get() -> %p"), waveManager);
		StartOrAdvanceWave(waveManager);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("DebugUI: OnButtonEnemyWaveClicked - WaveManager not found"));
}
AEnemyGroupSpawnPoint* UDebugUIWidget::SpawnDebugSpawnPoint(const FTransform& transform)
{
	UWorld* world = GetWorld();
	if (!world)
	{
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyGroupSpawnPoint* spawnPoint = world->SpawnActor<AEnemyGroupSpawnPoint>(AEnemyGroupSpawnPoint::StaticClass(), transform, spawnParams);

	if (!spawnPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDebugUIWidget::SpawnDebugSpawnPoint failed"));
		return nullptr;
	}

	return spawnPoint;
}

void UDebugUIWidget::SpawnEnemyInternal(TWeakObjectPtr<AEnemyGroupSpawnPoint> weakSpawnPoint,
	TSubclassOf<AUnit> enemyClass,
	int32 enemyIndex,
	FTransform fallbackTransform)
{
	if (!enemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnEnemyInternal: EnemyClass is null"));
		return;
	}

	UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	// Resolve final transform at spawn time: try spawnpoint first, else fallback
	FTransform finalTransform = fallbackTransform;
	if (weakSpawnPoint.IsValid())
	{
		AEnemyGroupSpawnPoint* spawnPoint = weakSpawnPoint.Get();
		if (spawnPoint)
		{
			finalTransform = spawnPoint->GetActorTransform();
		}
	}

	// Add offset per index to avoid exact overlap ( spacing along spawn forward ) 
	const float spacing = 120.0f;
	const FVector localOffset = FVector(spacing * static_cast<float>  (enemyIndex), 0.0f, 0.0f);
	const FVector worldOffset = finalTransform.GetRotation().RotateVector(localOffset);

	finalTransform.AddToTranslation(worldOffset);

	// Spawn actor
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AUnit* spawned = world->SpawnActor<AUnit>(enemyClass, finalTransform, spawnParams);

	const FString className = enemyClass ? enemyClass->GetName() : FString(TEXT("None"));

	if (spawned)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Yellow, FString::Printf( TEXT("Spawned: %s  ( class %s )  at %s"), *spawned->GetName(), *className,
				*spawned->GetActorLocation().ToString()));
		}

#if WITH_EDITOR
		DrawDebugSphere(world, spawned->GetActorLocation(), 32.0f, 8, FColor::Green, false, 6.0f);
#endif
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT("UDebugUIWidget::SpawnEnemyInternal failed to spawn actor  ( class=%s ) "), *className);
	}
}

void UDebugUIWidget::ClearActiveTimers()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	FTimerManager& timerManager = world->GetTimerManager();
	for (FTimerHandle& timerHandle : ActiveSpawnTimers)
	{
		if (timerHandle.IsValid())
		{
			timerManager.ClearTimer(timerHandle);
		}
	}

	ActiveSpawnTimers.Reset();
}

void UDebugUIWidget::NativeDestruct()
{
	ClearActiveTimers();

	// unbind selection manager delegate
	if (SelectionManager)
	{
		SelectionManager->OnSelectionChanged.RemoveDynamic(this, &UDebugUIWidget::HandleSelectionChanged);
		SelectionManager = nullptr;
	}

	// unbind buttons safely
	if (Button1) Button1->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButton1Clicked);
	if (Button2) Button2->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButton2Clicked);
	if (Button3) Button3->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButton3Clicked);
	if (Button4) Button4->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButton4Clicked);
	if (Button7) Button7->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButton7Clicked);
	if (ButtonEnemyWave) ButtonEnemyWave->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButtonEnemyWaveClicked);

	// unsubscribe from WaveManager if we subscribed
	if (WaveManagerPtr.IsValid() && bIsSubscribedToWaveManager)
	{
		if (AWaveManager* wm = WaveManagerPtr.Get())
		{
			wm->OnAllWavesCompleted.RemoveDynamic(this, &UDebugUIWidget::HandleAllWavesCompleted);
			UE_LOG(LogTemp, Log, TEXT("DebugUI: Unsubscribed from WaveManager::OnAllWavesCompleted in NativeDestruct"));
		}
		bIsSubscribedToWaveManager = false;
	}

	// clear cached pointer
	WaveManagerPtr.Reset();

	Super::NativeDestruct();
}

void UDebugUIWidget::FindAndCacheWaveManager()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugUI: FindAndCacheWaveManager - no World"));
		// If we previously had a cached manager, clear subscription
		if (WaveManagerPtr.IsValid() && bIsSubscribedToWaveManager)
		{
			if (AWaveManager* Old = WaveManagerPtr.Get())
			{
				Old->OnAllWavesCompleted.RemoveDynamic(this, &UDebugUIWidget::HandleAllWavesCompleted);
			}
			bIsSubscribedToWaveManager = false;
		}
		WaveManagerPtr.Reset();
		return;
	}

	AActor* foundActor = UGameplayStatics::GetActorOfClass(world, AWaveManager::StaticClass());
	AWaveManager* foundWaveManager = Cast<AWaveManager>(foundActor);

	if (foundWaveManager)
	{
		UE_LOG(LogTemp, Log, TEXT("DebugUI: Found WaveManager '%s' (Actor: %s)"),
			*foundWaveManager->GetName(), *foundWaveManager->GetPathName());

		// If we switched to a different manager, unsubscribe old one first
		if (WaveManagerPtr.IsValid() && WaveManagerPtr.Get() != foundWaveManager)
		{
			if (AWaveManager* Old = WaveManagerPtr.Get())
			{
				if (bIsSubscribedToWaveManager)
				{
					Old->OnAllWavesCompleted.RemoveDynamic(this, &UDebugUIWidget::HandleAllWavesCompleted);
					bIsSubscribedToWaveManager = false;
				}
			}
		}

		WaveManagerPtr = foundWaveManager;

		// Subscribe if not yet subscribed
		if (!bIsSubscribedToWaveManager)
		{
			foundWaveManager->OnAllWavesCompleted.AddDynamic(this, &UDebugUIWidget::HandleAllWavesCompleted);
			bIsSubscribedToWaveManager = true;
			UE_LOG(LogTemp, Log, TEXT("DebugUI: Subscribed to WaveManager::OnAllWavesCompleted"));
		}
	}
	else
	{
		// no manager found -> clear cached pointer and subscription flag
		if (WaveManagerPtr.IsValid() && bIsSubscribedToWaveManager)
		{
			if (AWaveManager* Old = WaveManagerPtr.Get())
			{
				Old->OnAllWavesCompleted.RemoveDynamic(this, &UDebugUIWidget::HandleAllWavesCompleted);
			}
		}
		WaveManagerPtr.Reset();
		bIsSubscribedToWaveManager = false;
		UE_LOG(LogTemp, Warning, TEXT("DebugUI: WaveManager not found in world"));
	}
}

bool UDebugUIWidget::Initialize()
{
	const ESlateVisibility newVis = bExtraButtonsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	const bool success = Super::Initialize();
	if ( !success )
	{
		return false;
	}

	// Привязка коллбеков кнопок.
	if ( Button1 )
	{
		Button1->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton1Clicked );
	}
	if ( Button2 )
	{
		Button2->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton2Clicked );
		Button2->SetVisibility( newVis );
	}
	if ( Button3 )
	{
		Button3->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton3Clicked );
		Button3->SetVisibility( newVis );
	}
	if ( Button4 )
	{
		Button4->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton4Clicked );
	}

	if ( Button7 )
	{
		Button7->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButton7Clicked );
		Button7->SetVisibility( ESlateVisibility::Visible ); // по умолчанию скрыта
	}
	if (ButtonEnemyWave)
	{
		ButtonEnemyWave->OnClicked.RemoveDynamic(this, &UDebugUIWidget::OnButtonEnemyWaveClicked);
		ButtonEnemyWave->OnClicked.AddDynamic( this, &UDebugUIWidget::OnButtonEnemyWaveClicked );
	}
	if ( UWorld* world = GetWorld() )
	{
		if ( ADebugPlayerController* debugPC = Cast <ADebugPlayerController> ( UGameplayStatics::GetPlayerController( world, 0 ) ) )
		{
			SelectionManager = debugPC->GetSelectionManager();

			if ( SelectionManager )
			{
				SelectionManager->OnSelectionChanged.RemoveDynamic(this, &UDebugUIWidget::HandleSelectionChanged);
				SelectionManager->OnSelectionChanged.AddDynamic ( this, &UDebugUIWidget::HandleSelectionChanged );
				HandleSelectionChanged();
			}
		}
	}

	if ( !SelectionManager && GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
		    -1, 2.0f, FColor::Red, TEXT( "UDebugUIWidget: SelectionManager is NULL in Initialize" )
		);
	}
	return true;
}

void UDebugUIWidget::HandleSelectionChanged()
{
	if ( !Button7 || !SelectionManager )
	{
		return;
	}

	ABuilding* selectedBuilding = SelectionManager->GetPrimarySelectedBuilding();
	const bool bHasSelectedBuilding = ( selectedBuilding != nullptr );

	const ESlateVisibility newVis = bHasSelectedBuilding ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	Button7->SetVisibility( newVis );
}

void UDebugUIWidget::UpdateExtraButtonsVisibility()
{
	const ESlateVisibility newVis = bExtraButtonsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if ( Button2 )
	{
		Button2->SetVisibility( newVis );
	}
	if ( Button3 )
	{
		Button3->SetVisibility( newVis );
	}
}

void UDebugUIWidget::InitSelectionManager( USelectionManagerComponent* InSelectionManager )
{
	SelectionManager = InSelectionManager;

	if ( !SelectionManager )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "UDebugUIWidget::InitSelectionManager: InSelectionManager is null" )
			);
		}
		return;
	}

	SelectionManager->OnSelectionChanged.AddDynamic( this, &UDebugUIWidget::HandleSelectionChanged );

	HandleSelectionChanged();
}
void UDebugUIWidget::HandleAllWavesCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("UDebugUIWidget::HandleAllWavesCompleted called"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, TEXT("All waves completed"));
	}

}