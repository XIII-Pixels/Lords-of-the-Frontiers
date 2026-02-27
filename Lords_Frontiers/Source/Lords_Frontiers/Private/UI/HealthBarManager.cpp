#include "Lords_Frontiers/Public/UI/HealthBarManager.h"
#include "Lords_Frontiers/Public/UI/Widgets/HealthBarWidget.h"
#include "Camera/StrategyCamera.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Building/Building.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Components/ProgressBar.h"
#include "GameFramework/PlayerController.h" 
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AHealthBarManager::AHealthBarManager()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultWorldOffset = FVector( 0.f, 0.f, 200.f );
	AutoReleaseSeconds = 10.f;
	MinUpdateInterval = 0.033f;
	CameraMoveThreshold = 1.0f;
	CameraFOVThreshold = 0.25f;
}

void AHealthBarManager::BeginPlay()
{
	Super::BeginPlay();

	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( PC->PlayerCameraManager )
		{
			LastCameraLocation_ = PC->PlayerCameraManager->GetCameraLocation();
			LastCameraRotation_ = PC->PlayerCameraManager->GetCameraRotation();
			LastCameraFOV_ = PC->PlayerCameraManager->GetFOVAngle();
		}

		if ( APawn* Pawn = PC->GetPawn() )
		{
			if ( AStrategyCamera* CamPawn = Cast<AStrategyCamera>( Pawn ) )
			{
				UCameraComponent* CamComp = CamPawn->Camera;
				if ( CamComp )
				{
					LastCameraOrthoWidth_ = CamComp->OrthoWidth;
				}
			}
		}

	}
}

void AHealthBarManager::Tick( float delta )
{
	Super::Tick( delta );

	UpdateAccumulator_ += delta;
    if ( MinUpdateInterval > 0.f && UpdateAccumulator_ < MinUpdateInterval )
        return;

    UWorld* world = GetWorld();
	if ( !world )
        return;

    APlayerController* PC = UGameplayStatics::GetPlayerController( world, 0 );
    if ( !PC )
        return;

    bool bCameraChanged = false;

    float currOrthoWidth = -1.f;

    // Detect camera position/rotation/FOV changes 
    if ( PC->PlayerCameraManager )
    {
        const FVector currCamLoc = PC->PlayerCameraManager->GetCameraLocation();
        const FRotator currCamRot = PC->PlayerCameraManager->GetCameraRotation();
        const float currFOV = PC->PlayerCameraManager->GetFOVAngle();

        if ( FVector::DistSquared( currCamLoc, LastCameraLocation_ ) > FMath::Square( CameraMoveThreshold ) )
        {
            bCameraChanged = true;
        }

        const float yawDiff = FMath::Abs( FRotator::NormalizeAxis( currCamRot.Yaw - LastCameraRotation_.Yaw ) );
		const float pitchDiff = FMath::Abs( FRotator::NormalizeAxis( currCamRot.Pitch - LastCameraRotation_.Pitch ) );
		if ( yawDiff > 0.1f || pitchDiff > 0.1f )
        {
            bCameraChanged = true;
        }

        if ( FMath::Abs( currFOV - LastCameraFOV_ ) > CameraFOVThreshold )
        {
            bCameraChanged = true;
        }
    }

    //  Detect orthographic zoom 
    if ( APawn* pawn = PC->GetPawn() )
    {
		if ( AStrategyCamera* camPawn = Cast<AStrategyCamera>( pawn ) )
        {
			UCameraComponent* camComp = camPawn->Camera;
			if ( camComp )
            {
				currOrthoWidth = camComp->OrthoWidth;

                if ( LastCameraOrthoWidth_ >= 0.f && FMath::Abs( currOrthoWidth - LastCameraOrthoWidth_ ) > CameraOrthoWidthThreshold )
                {
                    bCameraChanged = true;
                }
            }
        }
    }

    if ( bCameraChanged )
    {
        UpdateAllPositions();

        if ( PC->PlayerCameraManager )
        {
            LastCameraLocation_ = PC->PlayerCameraManager->GetCameraLocation();
            LastCameraRotation_ = PC->PlayerCameraManager->GetCameraRotation();
            LastCameraFOV_ = PC->PlayerCameraManager->GetFOVAngle();
        }

		if ( currOrthoWidth >= 0.f )
        {
			LastCameraOrthoWidth_ = currOrthoWidth;
        }

        UpdateAccumulator_ = 0.f;
        return;
    }

    const float moveThreshSq = FMath::Square( ActorMoveThreshold );
    bool bAnyActorUpdated = false;

    TArray<TWeakObjectPtr<AActor>> keys;
	ActiveWidgets_.GetKeys( keys );

    for ( TWeakObjectPtr<AActor> weakKey : keys )
    {
		if ( !weakKey.IsValid() )
        {
			if ( UHealthBarWidget** Wptr = ActiveWidgets_.Find( weakKey ) )
            {
                if ( UHealthBarWidget* widg = *Wptr )
                {
					ReleaseWidget( widg );
                }
            }
			ActiveWidgets_.Remove( weakKey );
			ActiveWidgetLastWorldPos_.Remove( weakKey );
			ReleaseTimers_.Remove( weakKey );
            continue;
        }

        AActor* actor = weakKey.Get();
		if ( !actor )
            continue;

        const FVector worldPos = actor->GetActorLocation() + GetOffsetForActor( actor );

        FVector* cachedPtr = ActiveWidgetLastWorldPos_.Find( actor );
		if ( !cachedPtr )
        {
			ActiveWidgetLastWorldPos_.Add( actor, worldPos );
			UpdatePositionForActor( actor );
			bAnyActorUpdated = true;
			continue;
		}

		if ( ( worldPos - *cachedPtr ).SizeSquared() > moveThreshSq )
		{
			UpdatePositionForActor( actor );
			ActiveWidgetLastWorldPos_[actor] = worldPos;
			bAnyActorUpdated = true;
		}
	}

	if ( bAnyActorUpdated )
	{
		UpdateAccumulator_ = 0.f;
	}
}

bool AHealthBarManager::HasCameraMovedSignificantly( APlayerController* PC ) const
{
	if ( !PC || !PC->PlayerCameraManager )
		return false;

	// position
	const FVector currLoc = PC->PlayerCameraManager->GetCameraLocation();
	if ( FVector::DistSquared( currLoc, LastCameraLocation_ ) > FMath::Square( CameraMoveThreshold ) )
		return true;

	// rotation
	const FRotator currRot = PC->PlayerCameraManager->GetCameraRotation();
	const float yawDiff = FMath::Abs( FRotator::NormalizeAxis( currRot.Yaw - LastCameraRotation_.Yaw ) );
	const float pitchDiff = FMath::Abs( FRotator::NormalizeAxis( currRot.Pitch - LastCameraRotation_.Pitch ) );
	if ( yawDiff > 0.1f || pitchDiff > 0.1f )
		return true;

	// FOV
	const float currFOV = PC->PlayerCameraManager->GetFOVAngle();
	if ( FMath::Abs( currFOV - LastCameraFOV_ ) > CameraFOVThreshold )
		return true;

	if ( APawn* pawn = PC->GetPawn() )
	{
		if ( AStrategyCamera* camPawn = Cast<AStrategyCamera>( pawn ) )
		{
			UCameraComponent* camComp = camPawn->Camera;
			if ( camComp && LastCameraOrthoWidth_ >= 0.f )
			{
				const float currOrtho = camComp->OrthoWidth;
				if ( FMath::Abs( currOrtho - LastCameraOrthoWidth_ ) > CameraOrthoWidthThreshold )
					return true;
			}
		}
	}

	return false;
}

UHealthBarWidget* AHealthBarManager::AcquireWidget()
{
	// pool
	if ( WidgetPool_.Num() > 0 )
	{
		UHealthBarWidget* w = WidgetPool_.Pop();
		if ( w )
		{
			w->SetVisibility( ESlateVisibility::Collapsed );
			return w;
		}
	}

	if ( !HealthBarWidgetClass )
	{
		UE_LOG( LogTemp, Warning, TEXT( "HealthBarManager: HealthBarWidgetClass not assigned!" ) );
		return nullptr;
	}

	UUserWidget* created = CreateWidget<UUserWidget>( GetWorld(), HealthBarWidgetClass );
	if ( !created )
	{
		UE_LOG( LogTemp, Warning, TEXT( "HealthBarManager: CreateWidget returned null" ) );
		return nullptr;
	}

	UHealthBarWidget* hbw = Cast<UHealthBarWidget>( created );
	if ( !hbw )
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "HealthBarManager: Created widget is not UHealthBarWidget (check WBP parent class)" )
		);
		created->AddToViewport( 1000 );
		created->SetVisibility( ESlateVisibility::Collapsed );
		return nullptr;
	}

	hbw->AddToViewport( 1000 );
	hbw->SetVisibility( ESlateVisibility::Collapsed );
	return hbw;
}

void AHealthBarManager::ReleaseWidget( UHealthBarWidget* widget )
{
	if ( !widget )
		return;
	widget->Unbind();
	widget->SetVisibility( ESlateVisibility::Collapsed );
	WidgetPool_.Add( widget );
}

FVector AHealthBarManager::GetOffsetForActor( AActor* actor ) const
{
	if ( !actor )
		return DefaultWorldOffset;


	if ( const FVector* override = ActorOffsets_.Find( actor ) )
	{
		if ( !override->IsNearlyZero() )
			return *override;
	}

	if ( const ABuilding* building = Cast<ABuilding>( actor ) )
	{
		if ( !building->HealthBarWorldOffset.IsNearlyZero() )
			return building->HealthBarWorldOffset;
	}
	else if ( const AUnit* unit = Cast<AUnit>( actor ) )
	{
		if ( !unit->HealthBarWorldOffset.IsNearlyZero() )
			return unit->HealthBarWorldOffset;
	}

	FBox bounds = actor->GetComponentsBoundingBox( false );
	const float height = bounds.GetExtent().Z;
	if ( height > KINDA_SMALL_NUMBER )
	{
		FVector Result = DefaultWorldOffset;
		Result.Z += height; // above actor
		return Result;
	}

	return DefaultWorldOffset;
}

void AHealthBarManager::RegisterActor( AActor* actor, const FVector& worldOffset /*=FVector::ZeroVector*/ )
{
	if ( !IsValid( actor ) )
		return;

	ActorOffsets_.FindOrAdd( actor ) = worldOffset;

	UE_LOG(
	    LogTemp, Verbose, TEXT( "HealthBarManager: RegisterActor offset for %s = %s" ), *GetNameSafe( actor ),
	    *worldOffset.ToString()
	);
}

void AHealthBarManager::UnregisterActor( AActor* actor )
{
	if ( !actor )
		return;

	ActorOffsets_.Remove( actor );

	if ( UHealthBarWidget** wPtr = ActiveWidgets_.Find( actor ) )
	{
		if ( UHealthBarWidget* widget = *wPtr )
		{
			ReleaseWidget( widget );
		}
		ActiveWidgets_.Remove( actor );
	}

	ActiveWidgetLastWorldPos_.Remove( actor );
	ReleaseTimers_.Remove( actor );

	UE_LOG( LogTemp, Verbose, TEXT( "HealthBarManager: UnregisterActor %s" ), *GetNameSafe( actor ) );
}

void AHealthBarManager::OnActorHealthChanged( AActor* actor, int32 current, int32 max )
{
	if ( !actor || !GetWorld() )
		return;

	if ( current <= 0 )
	{
		if ( UHealthBarWidget** foundDead = ActiveWidgets_.Find( actor ) )
		{
			UHealthBarWidget* wDead = *foundDead;
			if ( wDead )
			{
				if ( FTimerHandle* old = ReleaseTimers_.Find( actor ) )
				{
					GetWorld()->GetTimerManager().ClearTimer( *old );
					ReleaseTimers_.Remove( actor );
				}
				ActiveWidgetLastWorldPos_.Remove( actor );

				ReleaseWidget( wDead );
			}
			ActiveWidgets_.Remove( actor );
		}
		return;
	}

	if ( UHealthBarWidget** found = ActiveWidgets_.Find( actor ) )
	{
		UHealthBarWidget* w = *found;
		if ( w )
		{
			w->UpdateFromActor();
			w->ShowTemporary();
		}

		if ( FTimerHandle* old = ReleaseTimers_.Find( actor ) )
		{
			GetWorld()->GetTimerManager().ClearTimer( *old );
			ReleaseTimers_.Remove( actor );
		}
	}
	else
	{
		UHealthBarWidget* w = AcquireWidget();
		if ( !w )
			return;

		w->BindToActor( actor );

		FVector worldPos = actor->GetActorLocation() + GetOffsetForActor( actor );
		APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
		FVector2D screenPos;
		bool bProjected = pc && pc->ProjectWorldLocationToScreen( worldPos, screenPos, true );

		if ( !bProjected )
		{
			FVector2D vpSize( 1280.f, 720.f );
			if ( GEngine && GEngine->GameViewport )
				GEngine->GameViewport->GetViewportSize( vpSize );
			screenPos = vpSize * 0.5f;
		}

		w->SetAlignmentInViewport( FVector2D( 0.5f, 1.0f ) );
		w->SetPositionInViewport( screenPos, true );
		w->SetVisibility( ESlateVisibility::Visible );

		ActiveWidgets_.Add( actor, w );
		ActiveWidgetLastWorldPos_.Add( actor, worldPos );

		if ( current < max )
		{
			w->ShowTemporary();
		}

		if ( AutoReleaseSeconds > 0.f )
		{
			if ( FTimerHandle* old = ReleaseTimers_.Find( actor ) )
			{
				GetWorld()->GetTimerManager().ClearTimer( *old );
				ReleaseTimers_.Remove( actor );
			}

			FTimerHandle th;
			FTimerDelegate del = FTimerDelegate::CreateLambda(
			    [this, actor]()
			    {
				    if ( UHealthBarWidget** wP = ActiveWidgets_.Find( actor ) )
				    {
					    if ( UHealthBarWidget* wPtr = *wP )
					    {
						    ReleaseWidget( wPtr );
					    }
					    ActiveWidgets_.Remove( actor );
				    }
				    ReleaseTimers_.Remove( actor );
			    }
			);
			GetWorld()->GetTimerManager().SetTimer( th, del, AutoReleaseSeconds, false );
			ReleaseTimers_.Add( actor, th );
		}
	}
}

void AHealthBarManager::UpdateAllPositions()
{
	if ( !GetWorld() )
		return;

	APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
	if ( !pc )
		return;

	FVector2D viewportSize( 1280.f, 720.f );
	if ( GEngine && GEngine->GameViewport )
	{
		GEngine->GameViewport->GetViewportSize( viewportSize );
	}

	TArray<TWeakObjectPtr<AActor>> keys;
	ActiveWidgets_.GetKeys( keys );

	for ( TWeakObjectPtr<AActor> weakActor : keys )
	{
		if ( !weakActor.IsValid() )
		{
			if ( UHealthBarWidget** wPtr = ActiveWidgets_.Find( weakActor ) )
			{
				if ( UHealthBarWidget* w = *wPtr )
				{
					ReleaseWidget( w );
				}
			}

			ActiveWidgets_.Remove( weakActor );
			ActiveWidgetLastWorldPos_.Remove( weakActor );
			ReleaseTimers_.Remove( weakActor );
			continue;
		}

		AActor* actor = weakActor.Get();
		if ( !actor )
		{
			ActiveWidgets_.Remove( weakActor );
			ActiveWidgetLastWorldPos_.Remove( weakActor );
			ReleaseTimers_.Remove( weakActor );
			continue;
		}

		UHealthBarWidget** widgetPtr = ActiveWidgets_.Find( actor );
		if ( !widgetPtr )
		{
			ActiveWidgetLastWorldPos_.Remove( actor );
			ReleaseTimers_.Remove( actor );
			continue;
		}

		UHealthBarWidget* widget = *widgetPtr;
		if ( !widget )
		{
			ActiveWidgets_.Remove( actor );
			ActiveWidgetLastWorldPos_.Remove( actor );
			ReleaseTimers_.Remove( actor );
			continue;
		}

		const FVector worldPos = actor->GetActorLocation() + GetOffsetForActor( actor );

		FVector2D screenPos;
		const bool bProjected = pc->ProjectWorldLocationToScreen( worldPos, screenPos, true );

		if ( !bProjected )
		{
			screenPos.X = FMath::Clamp( screenPos.X, 0.0f, viewportSize.X );
			screenPos.Y = FMath::Clamp( screenPos.Y, 0.0f, viewportSize.Y );
		}
		else
		{
			screenPos.X = FMath::Clamp( screenPos.X, 0.0f, viewportSize.X );
			screenPos.Y = FMath::Clamp( screenPos.Y, 0.0f, viewportSize.Y );
		}

		widget->SetAlignmentInViewport( FVector2D( 0.5f, 1.0f ) );
		widget->SetPositionInViewport( screenPos, true );

		ActiveWidgetLastWorldPos_.FindOrAdd( actor ) = worldPos;
	}
}

void AHealthBarManager::ShowAllRegistered()
{
	TArray<AActor*> found;
	UGameplayStatics::GetAllActorsOfClass(
	    GetWorld(), AActor::StaticClass(), found
	);

	for ( AActor* a : found )
	{
		if ( !a )
			continue;
		if ( ActiveWidgets_.Contains( a ) )
			continue;
		OnActorHealthChanged( a, 0, 0 );
		if ( UHealthBarWidget** wPtr = ActiveWidgets_.Find( a ) )
		{
			if ( UHealthBarWidget* w = *wPtr )
			{
				w->SuspendAutoHide( true );
			}
		}
	}
}

void AHealthBarManager::HideAllRegistered()
{
	TArray<TWeakObjectPtr<AActor>> keys;
	ActiveWidgets_.GetKeys( keys );
	for ( TWeakObjectPtr<AActor> key : keys )
	{
		if ( AActor* actor = key.Get() )
		{
			if ( UHealthBarWidget** wPtr = ActiveWidgets_.Find( actor ) )
			{
				if ( UHealthBarWidget* w = *wPtr )
				{
					w->SuspendAutoHide( false );
					ReleaseWidget( w );
				}
			}
			ActiveWidgets_.Remove( actor );
		}
	}

	for ( auto& kv : ReleaseTimers_ )
	{
		if ( GetWorld() )
			GetWorld()->GetTimerManager().ClearTimer( kv.Value );
	}
	ReleaseTimers_.Empty();
}
void AHealthBarManager::ShowActiveWidgets()
{
	if ( !GetWorld() )
		return;

	APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 );

	ManagerForcedWidgets_.Empty();

	TArray<TWeakObjectPtr<AActor>> activeKeys;
	ActiveWidgets_.GetKeys( activeKeys );

	for ( TWeakObjectPtr<AActor> key : activeKeys )
	{
		if ( !key.IsValid() )
			continue;
		UHealthBarWidget* w = ActiveWidgets_.FindRef( key );
		if ( !w )
			continue;

		const bool bWasVisible = ( w->GetVisibility() == ESlateVisibility::Visible );

		w->SuspendAutoHide( true );
		w->UpdateFromActor();

		if ( !bWasVisible )
		{
			w->SetVisibility( ESlateVisibility::Visible );
			ManagerForcedWidgets_.Add( w );
		}

		if ( FTimerHandle* th = ReleaseTimers_.Find( key ) )
		{
			GetWorld()->GetTimerManager().ClearTimer( *th );
			ReleaseTimers_.Remove( key );
		}
	}

	//  HP < Max
	TArray<AActor*> found;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), ABuilding::StaticClass(), found );

	for ( AActor* actor : found )
	{
		if ( !actor )
			continue;

		ABuilding* building = Cast<ABuilding>( actor );
		if ( !building )
			continue;

		const int32 current = building->GetCurrentHealth();
		const int32 max = building->GetMaxHealth();

		if ( current >= max )
			continue;

		if ( ActiveWidgets_.Contains( actor ) )
			continue;

		UHealthBarWidget* w = AcquireWidget();
		if ( !w )
		{
			UE_LOG(
			    LogTemp, Warning, TEXT( "HealthBarManager::ShowActiveWidgets - AcquireWidget failed for actor %s" ),
			    *GetNameSafe( actor )
			);
			continue;
		}

		w->BindToActor( actor );

		FVector worldPos = actor->GetActorLocation() + GetOffsetForActor( actor );
		FVector2D screenPos;
		bool bProjected = ( pc && pc->ProjectWorldLocationToScreen( worldPos, screenPos, true ) );
		if ( !bProjected )
		{
			FVector2D vpSize( 1280.f, 720.f );
			if ( GEngine && GEngine->GameViewport )
				GEngine->GameViewport->GetViewportSize( vpSize );
			screenPos = vpSize * 0.5f;
		}

		w->SetAlignmentInViewport( FVector2D( 0.5f, 1.0f ) );
		w->SetPositionInViewport( screenPos, true );
		w->SetVisibility( ESlateVisibility::Visible );
		w->SuspendAutoHide( true );
		w->UpdateFromActor();

		ActiveWidgets_.Add( actor, w );
		ActiveWidgetLastWorldPos_.Add( actor, worldPos );

		if ( FTimerHandle* th = ReleaseTimers_.Find( actor ) )
		{
			GetWorld()->GetTimerManager().ClearTimer( *th );
			ReleaseTimers_.Remove( actor );
		}

		ManagerForcedWidgets_.Add( w );

		UE_LOG(
		    LogTemp, Verbose, TEXT( "HealthBarManager::ShowActiveWidgets - created/shown for %s (HP %d/%d)" ),
		    *GetNameSafe( actor ), current, max
		);
	}
}

void AHealthBarManager::HideActiveWidgets()
{
	if ( !GetWorld() )
		return;

	TArray<TWeakObjectPtr<AActor>> keys;
	ActiveWidgets_.GetKeys( keys );

	for ( TWeakObjectPtr<AActor> key : keys )
	{
		if ( !key.IsValid() )
		{
			if ( UHealthBarWidget* wDead = ActiveWidgets_.FindRef( key ) )
			{
				ReleaseWidget( wDead );
			}
			ActiveWidgets_.Remove( key );
			continue;
		}

		AActor* actor = key.Get();
		UHealthBarWidget* w = ActiveWidgets_.FindRef( key );
		if ( !w )
		{
			ActiveWidgets_.Remove( key );
			continue;
		}

		const bool bWasForced = ManagerForcedWidgets_.Contains( w );

		if ( bWasForced )
		{
			w->SuspendAutoHide( false );

			if ( FTimerHandle* th = ReleaseTimers_.Find( key ) )
			{
				GetWorld()->GetTimerManager().ClearTimer( *th );
				ReleaseTimers_.Remove( key );
			}

			ActiveWidgetLastWorldPos_.Remove( key );
			ReleaseWidget( w );
			ActiveWidgets_.Remove( key );
		}
		else
		{
			w->SuspendAutoHide( false );
			w->UpdateFromActor();
		}
	}

	ManagerForcedWidgets_.Empty();

	for ( auto& kv : ReleaseTimers_ )
	{
		GetWorld()->GetTimerManager().ClearTimer( kv.Value );
	}
	ReleaseTimers_.Empty();
}

void AHealthBarManager::UpdatePositionForActor( AActor* Actor )
{
	if ( !Actor || !GetWorld() )
		return;

	UHealthBarWidget** wPtr = ActiveWidgets_.Find( Actor );
	if ( !wPtr )
		return;

	UHealthBarWidget* w = *wPtr;
	if ( !w )
		return;

	APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
	if ( !pc )
		return;

	FVector worldPos = Actor->GetActorLocation() + GetOffsetForActor( Actor );
	FVector2D screenPos;
	bool bProjected = pc->ProjectWorldLocationToScreen( worldPos, screenPos, true );

	if ( !bProjected )
	{
		FVector2D vpSize( 1280.f, 720.f );
		if ( GEngine && GEngine->GameViewport )
			GEngine->GameViewport->GetViewportSize( vpSize );
		screenPos.X = FMath::Clamp( screenPos.X, 0.f, vpSize.X );
		screenPos.Y = FMath::Clamp( screenPos.Y, 0.f, vpSize.Y );
	}

	w->SetAlignmentInViewport( FVector2D( 0.5f, 1.0f ) );
	w->SetPositionInViewport( screenPos, true );

	ActiveWidgetLastWorldPos_.FindOrAdd( Actor ) = worldPos;
}