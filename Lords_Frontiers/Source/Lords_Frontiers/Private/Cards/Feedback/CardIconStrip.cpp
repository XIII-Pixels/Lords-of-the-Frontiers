#include "Cards/Feedback/CardIconStrip.h"

#include "Cards/Feedback/CardIconStripWidget.h"

#include "Components/WidgetComponent.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardIconStrip, Log, All );

ACardIconStrip::ACardIconStrip()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>( TEXT( "WidgetComponent" ) );
	RootComponent = WidgetComponent_;
	WidgetComponent_->SetWidgetSpace( EWidgetSpace::Screen );
	WidgetComponent_->SetDrawAtDesiredSize( false );
}

void ACardIconStrip::BeginPlay()
{
	Super::BeginPlay();

	EnsureWidgetInitialized();
	SetDormant( true );
}

void ACardIconStrip::EnsureWidgetInitialized()
{
	if ( !WidgetComponent_ || WidgetInstance_ )
	{
		return;
	}

	WidgetComponent_->SetDrawSize( WidgetDrawSize_ );

	if ( !WidgetClass_ )
	{
		UE_LOG( LogCardIconStrip, Warning,
			TEXT( "ACardIconStrip: WidgetClass not set on %s" ), *GetName() );
		return;
	}

	WidgetComponent_->SetWidgetClass( WidgetClass_ );
	WidgetComponent_->InitWidget();

	UUserWidget* userWidget = WidgetComponent_->GetUserWidgetObject();
	WidgetInstance_ = Cast<UCardIconStripWidget>( userWidget );
	if ( !WidgetInstance_ )
	{
		UE_LOG( LogCardIconStrip, Warning,
			TEXT( "ACardIconStrip: WidgetClass '%s' is not a UCardIconStripWidget subclass" ),
			*GetNameSafe( WidgetClass_ ) );
	}
}

void ACardIconStrip::ActivateOn( AActor* host )
{
	if ( !host )
	{
		return;
	}

	HostActor_ = host;
	SlotHeightOffsets_.Empty();
	CurrentHeightOffset_ = 0.f;

	EnsureWidgetInitialized();
	if ( WidgetInstance_ )
	{
		WidgetInstance_->ClearAll();
	}

	AttachToActor( host, FAttachmentTransformRules::KeepWorldTransform );
	SetActorRelativeLocation( FVector::ZeroVector );
	SetDormant( false );
}

void ACardIconStrip::DeactivateToPool()
{
	if ( !bIsActive_ )
	{
		return;
	}

	if ( WidgetInstance_ )
	{
		WidgetInstance_->ClearAll();
	}

	SlotHeightOffsets_.Empty();
	CurrentHeightOffset_ = 0.f;
	HostActor_.Reset();

	DetachFromActor( FDetachmentTransformRules::KeepRelativeTransform );
	SetDormant( true );
}

int32 ACardIconStrip::AddIconSlot( UTexture2D* icon, float requestedHeightOffset )
{
	EnsureWidgetInitialized();
	if ( !WidgetInstance_ )
	{
		return INDEX_NONE;
	}

	const int32 widgetSlotId = WidgetInstance_->AddIcon( icon );
	if ( widgetSlotId == INDEX_NONE )
	{
		return INDEX_NONE;
	}

	SlotHeightOffsets_.Add( widgetSlotId, requestedHeightOffset );
	ApplyHeightOffset();
	return widgetSlotId;
}

bool ACardIconStrip::RemoveIconSlot( int32 slotId, float releasedHeightOffset )
{
	if ( WidgetInstance_ )
	{
		WidgetInstance_->RemoveIcon( slotId );
	}
	SlotHeightOffsets_.Remove( slotId );

	ApplyHeightOffset();

	return SlotHeightOffsets_.Num() == 0;
}

int32 ACardIconStrip::GetSlotCount() const
{
	return WidgetInstance_ ? WidgetInstance_->GetSlotCount() : SlotHeightOffsets_.Num();
}

void ACardIconStrip::ApplyHeightOffset()
{
	float maxOffset = 0.f;
	for ( const TPair<int32, float>& pair : SlotHeightOffsets_ )
	{
		maxOffset = FMath::Max( maxOffset, pair.Value );
	}
	CurrentHeightOffset_ = maxOffset;
	SetActorRelativeLocation( FVector( 0.f, 0.f, CurrentHeightOffset_ ) );
}

void ACardIconStrip::SetDormant( bool bDormant )
{
	bIsActive_ = !bDormant;
	SetActorHiddenInGame( bDormant );
	SetActorEnableCollision( false );
}
