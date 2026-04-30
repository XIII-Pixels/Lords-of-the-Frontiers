#include "Cards/StatusEffects/StatusEffectTracker.h"

#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/Visuals/CardVisualSubsystem.h"

#include "Engine/World.h"

UStatusEffectTracker::UStatusEffectTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UStatusEffectTracker::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	ClearAll();
	Super::EndPlay( endPlayReason );
}

void UStatusEffectTracker::TickComponent(
	float dt, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction )
{
	Super::TickComponent( dt, tickType, thisTickFunction );

	if ( Active_.Num() == 0 )
	{
		return;
	}

	AActor* owner = GetOwner();
	UWorld* world = GetWorld();
	if ( !owner || !world )
	{
		return;
	}
	const float now = world->GetTimeSeconds();

	TArray<FName> keys;
	Active_.GenerateKeyArray( keys );

	TArray<FName> expired;
	for ( const FName& key : keys )
	{
		FActiveStatus* state = Active_.Find( key );
		if ( !state )
		{
			continue;
		}
		if ( !state->Def )
		{
			expired.Add( key );
			continue;
		}

		if ( state->Def->GetTickInterval() > 0.f && now >= state->NextTickAt )
		{
			state->Def->OnTick( owner, *state );

			if ( !IsValid( this ) || !IsValid( owner ) )
			{
				return;
			}

			state = Active_.Find( key );
			if ( !state || !state->Def )
			{
				continue;
			}
			state->NextTickAt = now + state->Def->GetTickInterval();
		}

		if ( now >= state->ExpiresAt )
		{
			expired.Add( key );
		}
	}

	for ( const FName& key : expired )
	{
		if ( FActiveStatus* state = Active_.Find( key ) )
		{
			if ( state->Def )
			{
				state->Def->OnRemove( owner, *state );
			}
			ReleaseStatusVisual( *state );
			Active_.Remove( key );
		}
	}
}

void UStatusEffectTracker::ApplyStatus( UStatusEffectDef* def, AActor* instigator )
{
	if ( !def || def->StatusTag.IsNone() )
	{
		return;
	}

	const float now = GetWorld()->GetTimeSeconds();

	if ( FActiveStatus* existing = Active_.Find( def->StatusTag ) )
	{
		existing->ExpiresAt = now + def->Duration;
		if ( instigator )
		{
			existing->Instigator = instigator;
		}
		def->OnReapply( GetOwner(), *existing );
		return;
	}

	FActiveStatus state;
	state.Def = def;
	state.ExpiresAt = now + def->Duration;
	state.NextTickAt = def->GetTickInterval() > 0.f ? now + def->GetTickInterval() : MAX_flt;
	state.Instigator = instigator;

	def->OnApply( GetOwner(), state );

	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		state.VisualHandle = visuals->BeginSticky( def->VisualConfig, GetOwner(), nullptr );
	}

	Active_.Add( def->StatusTag, state );
}

bool UStatusEffectTracker::HasStatus( const UStatusEffectDef* def ) const
{
	if ( !def || def->StatusTag.IsNone() )
	{
		return false;
	}
	return Active_.Contains( def->StatusTag );
}

bool UStatusEffectTracker::HasStatusTag( FName tag ) const
{
	return !tag.IsNone() && Active_.Contains( tag );
}

void UStatusEffectTracker::RemoveStatus( UStatusEffectDef* def )
{
	if ( !def )
	{
		return;
	}

	if ( FActiveStatus* state = Active_.Find( def->StatusTag ) )
	{
		if ( state->Def )
		{
			state->Def->OnRemove( GetOwner(), *state );
		}
		ReleaseStatusVisual( *state );
		Active_.Remove( def->StatusTag );
	}
}

void UStatusEffectTracker::ClearAll()
{
	AActor* owner = GetOwner();
	for ( auto& pair : Active_ )
	{
		if ( pair.Value.Def )
		{
			pair.Value.Def->OnRemove( owner, pair.Value );
		}
		ReleaseStatusVisual( pair.Value );
	}
	Active_.Empty();
}

void UStatusEffectTracker::ReleaseStatusVisual( FActiveStatus& state )
{
	if ( !state.VisualHandle.IsValid() )
	{
		return;
	}

	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		visuals->EndSticky( state.VisualHandle );
	}
	state.VisualHandle.Reset();
}

UStatusEffectTracker* UStatusEffectTracker::EnsureOn( AActor* actor )
{
	if ( !actor )
	{
		return nullptr;
	}

	if ( UStatusEffectTracker* existing = actor->FindComponentByClass<UStatusEffectTracker>() )
	{
		return existing;
	}

	UStatusEffectTracker* tracker = NewObject<UStatusEffectTracker>( actor );
	if ( tracker )
	{
		tracker->RegisterComponent();
	}
	return tracker;
}
