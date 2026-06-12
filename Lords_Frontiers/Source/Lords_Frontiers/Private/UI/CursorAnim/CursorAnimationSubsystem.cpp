#include "UI/CursorAnim/CursorAnimationSubsystem.h"

#include "UI/CursorAnim/CursorAnimationConfig.h"
#include "UI/CursorAnim/CursorAnimationPlayerWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UCursorAnimationSubsystem* UCursorAnimationSubsystem::Get( const UObject* worldContextObject )
{
	if ( !worldContextObject )
	{
		return nullptr;
	}
	const UWorld* world = GEngine->GetWorldFromContextObject(
	    worldContextObject, EGetWorldErrorMode::ReturnNull
	);
	if ( !world )
	{
		return nullptr;
	}
	return world->GetSubsystem<UCursorAnimationSubsystem>();
}

void UCursorAnimationSubsystem::Deinitialize()
{
	for ( const TObjectPtr<UCursorAnimationPlayerWidget>& player : InUse_ )
	{
		if ( player )
		{
			player->RemoveFromParent();
		}
	}
	InUse_.Reset();
	FreePool_.Reset();

	Super::Deinitialize();
}

void UCursorAnimationSubsystem::SetConfig( UCursorAnimationConfig* config )
{
	Config_ = config;
}

void UCursorAnimationSubsystem::PlayAtScreenPosition( const FGameplayTag& tag, const FVector2D& screenPosition )
{
	if ( !Config_ )
	{
		return;
	}

	const FCursorAnimationEntry* entry = Config_->FindByTag( tag );
	if ( !entry )
	{
		return;
	}

	UCursorAnimationPlayerWidget* player = AcquirePlayer();
	if ( !player )
	{
		return;
	}

	player->AddToViewport( Config_->ViewportZOrder );
	player->SetAlignmentInViewport( FVector2D( 0.5f, 0.5f ) );

	FVector2D pixelPosition = FVector2D::ZeroVector;
	FVector2D viewportPosition = FVector2D::ZeroVector;
	USlateBlueprintLibrary::AbsoluteToViewport( this, screenPosition, pixelPosition, viewportPosition );
	player->SetPositionInViewport( viewportPosition, false );

	player->Play( *entry );

	InUse_.Add( player );
}

UCursorAnimationPlayerWidget* UCursorAnimationSubsystem::AcquirePlayer()
{
	while ( FreePool_.Num() > 0 )
	{
		UCursorAnimationPlayerWidget* pooled = FreePool_.Pop();
		if ( pooled )
		{
			return pooled;
		}
	}

	if ( !Config_ || !Config_->PlayerWidgetClass )
	{
		return nullptr;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return nullptr;
	}
	APlayerController* pc = world->GetFirstPlayerController();
	if ( !pc )
	{
		return nullptr;
	}

	UCursorAnimationPlayerWidget* player =
	    CreateWidget<UCursorAnimationPlayerWidget>( pc, Config_->PlayerWidgetClass );
	if ( player )
	{
		player->OnFinished.AddUObject( this, &UCursorAnimationSubsystem::HandlePlayerFinished );
	}
	return player;
}

void UCursorAnimationSubsystem::HandlePlayerFinished( UCursorAnimationPlayerWidget* player )
{
	if ( !player )
	{
		return;
	}
	player->RemoveFromParent();
	InUse_.Remove( player );
	FreePool_.Add( player );
}
