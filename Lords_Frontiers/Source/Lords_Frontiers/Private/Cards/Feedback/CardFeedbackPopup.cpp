#include "Cards/Feedback/CardFeedbackPopup.h"

#include "Cards/Feedback/CardFeedbackWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardFeedbackPopup, Log, All );

ACardFeedbackPopup::ACardFeedbackPopup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>( TEXT( "WidgetComponent" ) );
	RootComponent = WidgetComponent_;
	WidgetComponent_->SetWidgetSpace( EWidgetSpace::Screen );
	WidgetComponent_->SetDrawAtDesiredSize( false );
}

void ACardFeedbackPopup::BeginPlay()
{
	Super::BeginPlay();

	if ( !WidgetComponent_ )
	{
		return;
	}

	WidgetComponent_->SetDrawSize( WidgetDrawSize_ );

	if ( !WidgetClass_ )
	{
		UE_LOG( LogCardFeedbackPopup, Warning,
			TEXT( "ACardFeedbackPopup: WidgetClass is not set on %s" ),
			*GetName() );
		return;
	}

	WidgetComponent_->SetWidgetClass( WidgetClass_ );
	WidgetComponent_->InitWidget();

	UUserWidget* userWidget = WidgetComponent_->GetUserWidgetObject();
	WidgetInstance_ = Cast<UCardFeedbackWidget>( userWidget );

	if ( !WidgetInstance_ )
	{
		UE_LOG( LogCardFeedbackPopup, Warning,
			TEXT( "ACardFeedbackPopup: WidgetClass '%s' is not a UCardFeedbackWidget subclass" ),
			*GetNameSafe( WidgetClass_ ) );
	}

	SetDormant( true );
}

void ACardFeedbackPopup::ActivatePopup(
	AActor* owner, UTexture2D* icon, float durationSeconds, float floatHeight, float baseHeightOffset )
{
	if ( !owner || !icon )
	{
		return;
	}

	Mode_ = ECardFeedbackPopupMode::Popup;
	OwnerActor_ = owner;
	Duration_ = FMath::Max( durationSeconds, 0.05f );
	FloatHeight_ = floatHeight;
	BaseHeightOffset_ = baseHeightOffset;
	Elapsed_ = 0.f;

	AttachToActor( owner, FAttachmentTransformRules::KeepWorldTransform );
	SetActorRelativeLocation( FVector( 0.f, 0.f, BaseHeightOffset_ ) );

	ApplyIcon( icon );
	SetDormant( false );
}

void ACardFeedbackPopup::ActivateSticky( AActor* owner, UTexture2D* icon, float baseHeightOffset )
{
	if ( !owner || !icon )
	{
		return;
	}

	Mode_ = ECardFeedbackPopupMode::Sticky;
	OwnerActor_ = owner;
	BaseHeightOffset_ = baseHeightOffset;
	Elapsed_ = 0.f;

	AttachToActor( owner, FAttachmentTransformRules::KeepWorldTransform );
	SetActorRelativeLocation( FVector( 0.f, 0.f, BaseHeightOffset_ ) );

	ApplyIcon( icon );
	SetDormant( false );
}

void ACardFeedbackPopup::DeactivateToPool()
{
	if ( !bIsActive_ )
	{
		return;
	}

	OwnerActor_.Reset();
	Elapsed_ = 0.f;
	DetachFromActor( FDetachmentTransformRules::KeepRelativeTransform );

	if ( WidgetInstance_ )
	{
		WidgetInstance_->UpdateAlpha( 0.f );
	}

	SetDormant( true );
}

void ACardFeedbackPopup::ApplyIcon( UTexture2D* icon )
{
	if ( !WidgetInstance_ || !icon )
	{
		return;
	}

	WidgetInstance_->SetIcon( icon );
	WidgetInstance_->UpdateAlpha( 1.f );
	WidgetInstance_->OnShow();
}

void ACardFeedbackPopup::SetDormant( bool bDormant )
{
	bIsActive_ = !bDormant;
	SetActorHiddenInGame( bDormant );
	SetActorTickEnabled( !bDormant );
	SetActorEnableCollision( false );
}

void ACardFeedbackPopup::Tick( float deltaTime )
{
	Super::Tick( deltaTime );

	if ( !bIsActive_ )
	{
		return;
	}

	if ( !OwnerActor_.IsValid() )
	{
		OnPopupFinished.Broadcast( this );
		return;
	}

	if ( Mode_ != ECardFeedbackPopupMode::Popup )
	{
		return;
	}

	Elapsed_ += deltaTime;

	const float t = FMath::Clamp( Elapsed_ / Duration_, 0.f, 1.f );
	SetActorRelativeLocation( FVector( 0.f, 0.f, BaseHeightOffset_ + FMath::Lerp( 0.f, FloatHeight_, t ) ) );

	if ( WidgetInstance_ )
	{
		WidgetInstance_->UpdateAlpha( 1.f - t );
	}

	if ( Elapsed_ >= Duration_ )
	{
		OnPopupFinished.Broadcast( this );
	}
}
