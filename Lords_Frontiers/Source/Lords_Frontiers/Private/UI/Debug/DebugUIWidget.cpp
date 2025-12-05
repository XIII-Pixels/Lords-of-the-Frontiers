// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/UI/Debug/DebugUIWidget.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"



void UDebugUIWidget::OnButton1Clicked () 
{
	GEngine->AddOnScreenDebugMessage ( -1, 5.0f, FColor::Red, "Button 1 Clicked" );
}

void UDebugUIWidget::OnButton2Clicked () 
{
	GEngine->AddOnScreenDebugMessage ( -1, 5.0f, FColor::Red, "Button 2 Clicked" );
}

void UDebugUIWidget::OnButton3Clicked () 
{
	GEngine->AddOnScreenDebugMessage ( -1, 5.0f, FColor::Red, "Button 3 Clicked" );
}

void UDebugUIWidget::OnButton4Clicked () 
{
	if  ( GEngine ) 
	{
		GEngine->AddOnScreenDebugMessage ( -1, 3.0f, FColor::Red, TEXT ( "Button4 clicked - spawn/wave trigger DEBUG" )  );
	}

	UE_LOG ( LogTemp, Log, TEXT ( "DebugUI: OnButton4Clicked - WaveManagerPtr.IsValid ()  = %d" ) ,
		WaveManagerPtr.IsValid ()  ? 1 : 0 );

	// start wave 0  ( first wave ) 
	if  ( WaveManagerPtr.IsValid ()  ) 
	{
		AWaveManager* waveManager = WaveManagerPtr.Get ();
		UE_LOG ( LogTemp, Log, TEXT ( "DebugUI: WaveManagerPtr.Get ()  -> %p" ) , waveManager );

		if  ( waveManager ) 
		{
			if  ( waveManager->IsFirstWaveRequested ()  == false ) 
			{
				waveManager->StartWaveAtIndex ( 0 );
			}
			else
			{
				waveManager->AdvanceToNextWave ();
			}

			UE_LOG ( LogTemp, Log, TEXT ( "DebugUI: requested WaveManager to start NEXT wave" )  );
			return; // done — do not run debug immediate spawn
		}
	}
	else
	{
		// Try to find again in case widget was created before WaveManager existed
		FindAndCacheWaveManager ();
		if  ( WaveManagerPtr.IsValid ()  ) 
		{
			AWaveManager* waveManager = WaveManagerPtr.Get ();
			if  ( waveManager ) 
			{
				if  ( waveManager->IsFirstWaveRequested ()  == false ) 
				{
					waveManager->StartWaveAtIndex ( 0 );
				}
				else
				{
					waveManager->AdvanceToNextWave ();
				}

				UE_LOG ( LogTemp, Log, TEXT ( "DebugUI: requested WaveManager to start wave IN ELSE BLOCK" )  );
			}
		}
	}
}

AEnemyGroupSpawnPoint* UDebugUIWidget::SpawnDebugSpawnPoint ( const FTransform& transform ) 
{
	UWorld* world = GetWorld ();
	if  ( !world ) 
	{
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyGroupSpawnPoint* spawnPoint =
		world->SpawnActor<AEnemyGroupSpawnPoint> ( AEnemyGroupSpawnPoint::StaticClass () , transform, spawnParams );

	if  ( !spawnPoint ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT ( "UDebugUIWidget::SpawnDebugSpawnPoint failed" ) );
		return nullptr;
	}

	return spawnPoint;
}

void UDebugUIWidget::SpawnEnemyInternal  (  TWeakObjectPtr<AEnemyGroupSpawnPoint> weakSpawnPoint,
	TSubclassOf<AUnit> enemyClass,
	int32 enemyIndex,
	FTransform fallbackTransform ) 
{
	if  ( !enemyClass ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT ( "SpawnEnemyInternal: EnemyClass is null" )  );
		return;
	}

	UWorld* world = GetWorld ();
	if  ( !world ) 
	{
		return;
	}

	// Resolve final transform at spawn time: try spawnpoint first, else fallback
	FTransform finalTransform = fallbackTransform;
	if  ( weakSpawnPoint.IsValid ()  ) 
	{
		AEnemyGroupSpawnPoint* spawnPoint = weakSpawnPoint.Get ();
		if  ( spawnPoint ) 
		{
			finalTransform = spawnPoint->GetActorTransform ();
		}
	}

	// Add offset per index to avoid exact overlap  ( spacing along spawn forward ) 
	const float spacing = 120.0f;
	const FVector localOffset =
		FVector ( spacing * static_cast<float>  ( enemyIndex ) , 0.0f, 0.0f );
	const FVector worldOffset = finalTransform.GetRotation () .RotateVector ( localOffset );

	finalTransform.AddToTranslation ( worldOffset );

	// Spawn actor
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AUnit* spawned =
		world->SpawnActor<AUnit> ( enemyClass, finalTransform, spawnParams );

	const FString className = enemyClass ? enemyClass->GetName ()  : FString ( TEXT ( "None" )  );

	if  ( spawned ) 
	{
		if  ( GEngine ) 
		{
			GEngine->AddOnScreenDebugMessage ( 
				-1,
				3.0f,
				FColor::Yellow,
				FString::Printf ( 
					TEXT ( "Spawned: %s  ( class %s )  at %s" ) ,
					*spawned->GetName () ,
					*className,
					*spawned->GetActorLocation () .ToString ()  )  );
		}

#if WITH_EDITOR
		DrawDebugSphere ( world, spawned->GetActorLocation () , 32.0f, 8, FColor::Green, false, 6.0f );
#endif
	}
	else
	{
		UE_LOG ( 
			LogTemp,
			Warning,
			TEXT ( "UDebugUIWidget::SpawnEnemyInternal failed to spawn actor  ( class=%s ) " ) ,
			*className );
	}
}

void UDebugUIWidget::ClearActiveTimers () 
{
	UWorld* world = GetWorld ();
	if  ( !world ) 
	{
		return;
	}

	FTimerManager& timerManager = world->GetTimerManager ();
	for  ( FTimerHandle& timerHandle : ActiveSpawnTimers ) 
	{
		if  ( timerHandle.IsValid ()  ) 
		{
			timerManager.ClearTimer ( timerHandle );
		}
	}

	ActiveSpawnTimers.Reset ();
}

bool UDebugUIWidget::Initialize () 
{
	bool success = Super::Initialize ();
	if  ( !success ) 
	{
		return false;
	}

	if  ( Button1 ) 
	{
		Button1->OnClicked.AddDynamic ( this, &UDebugUIWidget::OnButton1Clicked );
	}
	if  ( Button2 ) 
	{
		Button2->OnClicked.AddDynamic ( this, &UDebugUIWidget::OnButton2Clicked );
	}
	if  ( Button3 ) 
	{
		Button3->OnClicked.AddDynamic ( this, &UDebugUIWidget::OnButton3Clicked );
	}
	if  ( Button4 ) 
	{
		Button4->OnClicked.AddDynamic ( this, &UDebugUIWidget::OnButton4Clicked );
	}

	FindAndCacheWaveManager ();

	return true;
}

void UDebugUIWidget::NativeDestruct () 
{
	ClearActiveTimers ();
	Super::NativeDestruct ();
}

void UDebugUIWidget::FindAndCacheWaveManager () 
{
	UWorld* world = GetWorld ();
	if  ( !world ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT ( "DebugUI: FindAndCacheWaveManager - no World" )  );
		WaveManagerPtr.Reset ();
		return;
	}

	AActor* foundActor =
		UGameplayStatics::GetActorOfClass ( world, AWaveManager::StaticClass ()  );
	AWaveManager* foundWaveManager = Cast<AWaveManager> ( foundActor );

	if  ( foundWaveManager ) 
	{
		UE_LOG ( 
			LogTemp,
			Log,
			TEXT ( "DebugUI: Found WaveManager '%s'  ( Actor: %s ) " ) ,
			*foundWaveManager->GetName () ,
			*foundWaveManager->GetPathName ()  );

		WaveManagerPtr = foundWaveManager;
	}
	else
	{
		WaveManagerPtr.Reset ();
		UE_LOG ( LogTemp, Warning, TEXT ( "DebugUI: WaveManager not found in world" )  );
	}
}

