#include "Cards/Feedback/CardFeedbackPopup.h"

#include "Cards/Feedback/CardFeedbackWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardFeedbackPopup, Log, All );

ACardFeedbackPopup::ACardFeedbackPopup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>( TEXT( "WidgetComponent" ) );
	RootComponent = WidgetComponent_;
	WidgetComponent_->SetWidgetSpace( EWidgetSpace::Screen );
	WidgetComponent_->SetDrawAtDesiredSize( false );
}

void ACardFeedbackPopup::Initialize( AActor* owner, UTexture2D* icon, float durationSeconds, float floatHeight )
{
	OwnerActor_ = owner;
	Duration_ = FMath::Max( durationSeconds, 0.05f );
	FloatHeight_ = floatHeight;
	Elapsed_ = 0.f;

	if ( owner )
	{
		AttachToActor( owner, FAttachmentTransformRules::KeepWorldTransform );
		SetActorRelativeLocation( FVector::ZeroVector );
	}

	if ( !WidgetComponent_ )
	{
		Destroy();
		return;
	}

	WidgetComponent_->SetDrawSize( WidgetDrawSize_ );

	if ( !WidgetClass_ )
	{
		UE_LOG( LogCardFeedbackPopup, Warning,
			TEXT( "ACardFeedbackPopup: WidgetClass is not set on %s — destroy popup" ),
			*GetName() );
		Destroy();
		return;
	}

	WidgetComponent_->SetWidgetClass( WidgetClass_ );
	WidgetComponent_->InitWidget();

	UUserWidget* userWidget = WidgetComponent_->GetUserWidgetObject();
	WidgetInstance_ = Cast<UCardFeedbackWidget>( userWidget );

	if ( !WidgetInstance_ )
	{
		UE_LOG( LogCardFeedbackPopup, Warning,
			TEXT( "ACardFeedbackPopup: WidgetClass '%s' is not a UCardFeedbackWidget subclass — destroy popup. Fix WBP parent." ),
			*GetNameSafe( WidgetClass_ ) );
		Destroy();
		return;
	}

	if ( icon )
	{
		WidgetInstance_->SetIcon( icon );
	}
	else
	{
		Destroy();
	}
}

void ACardFeedbackPopup::BeginPlay()
{
	Super::BeginPlay();
}

void ACardFeedbackPopup::Tick( float deltaTime )
{
	Super::Tick( deltaTime );

	if ( !OwnerActor_.IsValid() )
	{
		Destroy();
		return;
	}

	Elapsed_ += deltaTime;

	const float t = FMath::Clamp( Elapsed_ / Duration_, 0.f, 1.f );

	SetActorRelativeLocation( FVector( 0.f, 0.f, FMath::Lerp( 0.f, FloatHeight_, t ) ) );

	if ( WidgetInstance_ )
	{
		WidgetInstance_->UpdateAlpha( 1.f - t );
	}

	if ( Elapsed_ >= Duration_ )
	{
		Destroy();
	}
}
