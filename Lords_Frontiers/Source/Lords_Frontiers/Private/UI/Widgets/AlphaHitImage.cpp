#include "UI/Widgets/AlphaHitImage.h"

#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"

void UAlphaHitImage::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if ( TickHandle_.IsValid() )
	{
		return;
	}
	TickHandle_ = FTSTicker::GetCoreTicker().AddTicker(
	    FTickerDelegate::CreateUObject( this, &UAlphaHitImage::TickUpdate ), 0.0f
	);
}

void UAlphaHitImage::ReleaseSlateResources( bool bReleaseChildren )
{
	Super::ReleaseSlateResources( bReleaseChildren );
	StopTicker();
}

void UAlphaHitImage::BeginDestroy()
{
	StopTicker();
	Super::BeginDestroy();
}

void UAlphaHitImage::StopTicker()
{
	if ( !TickHandle_.IsValid() )
	{
		return;
	}
	FTSTicker::GetCoreTicker().RemoveTicker( TickHandle_ );
	TickHandle_.Reset();
}

bool UAlphaHitImage::TickUpdate( float /*deltaTime*/ )
{
	if ( !IsValid( this ) )
	{
		return false;
	}
	UpdateHitVisibility();
	return true;
}

void UAlphaHitImage::UpdateHitVisibility()
{
	TSharedPtr<SWidget> safeWidget = GetCachedWidget();
	if ( !safeWidget.IsValid() )
	{
		return;
	}

	EnsurePixelCache();
	if ( CachedPixels_.Num() == 0 || CachedSize_.X <= 0 || CachedSize_.Y <= 0 )
	{
		SetVisibility( ESlateVisibility::Visible );
		return;
	}

	const FGeometry& geo = safeWidget->GetCachedGeometry();
	const FVector2D localSize = geo.GetLocalSize();
	if ( localSize.X <= 0.0f || localSize.Y <= 0.0f )
	{
		return;
	}

	const FVector2D cursorAbs = FSlateApplication::Get().GetCursorPos();
	const FVector2D localPos = geo.AbsoluteToLocal( cursorAbs );

	if ( localPos.X < 0.0f || localPos.Y < 0.0f || localPos.X >= localSize.X || localPos.Y >= localSize.Y )
	{
		SetVisibility( ESlateVisibility::SelfHitTestInvisible );
		return;
	}

	const int32 texX = FMath::Clamp(
	    FMath::FloorToInt( ( localPos.X / localSize.X ) * CachedSize_.X ), 0, CachedSize_.X - 1
	);
	const int32 texY = FMath::Clamp(
	    FMath::FloorToInt( ( localPos.Y / localSize.Y ) * CachedSize_.Y ), 0, CachedSize_.Y - 1
	);

	const float alpha = CachedPixels_[ texY * CachedSize_.X + texX ].A / 255.0f;

	const ESlateVisibility newVis =
	    ( alpha >= AlphaThreshold ) ? ESlateVisibility::Visible : ESlateVisibility::SelfHitTestInvisible;

	if ( GetVisibility() != newVis )
	{
		SetVisibility( newVis );
	}
}

void UAlphaHitImage::EnsurePixelCache()
{
	UTexture2D* tex = Cast<UTexture2D>( GetBrush().GetResourceObject() );
	if ( !tex )
	{
		CachedTexture_ = nullptr;
		CachedPixels_.Reset();
		CachedSize_ = FIntPoint::ZeroValue;
		return;
	}
	if ( CachedTexture_ == tex && CachedPixels_.Num() > 0 )
	{
		return;
	}

	CachedTexture_ = tex;
	CachedPixels_.Reset();
	CachedSize_ = FIntPoint::ZeroValue;

	FTexturePlatformData* platformData = tex->GetPlatformData();
	if ( !platformData || platformData->Mips.Num() == 0 )
	{
		return;
	}

	FTexture2DMipMap& mip = platformData->Mips[ 0 ];
	const int32 width = mip.SizeX;
	const int32 height = mip.SizeY;
	if ( width <= 0 || height <= 0 )
	{
		return;
	}

	const void* data = mip.BulkData.LockReadOnly();
	if ( !data )
	{
		return;
	}

	CachedSize_ = FIntPoint( width, height );
	CachedPixels_.SetNumUninitialized( width * height );
	FMemory::Memcpy( CachedPixels_.GetData(), data, width * height * sizeof( FColor ) );
	mip.BulkData.Unlock();
}
