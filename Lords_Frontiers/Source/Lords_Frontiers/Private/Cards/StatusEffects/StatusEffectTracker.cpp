#include "Cards/StatusEffects/StatusEffectTracker.h"

#include "Cards/StatusEffects/StatusEffectDef.h"

#include "Engine/World.h"

UStatusEffectTracker::UStatusEffectTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
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
	const float now = GetWorld()->GetTimeSeconds();

	TArray<FName> expired;
	for ( auto& pair : Active_ )
	{
		FActiveStatus& state = pair.Value;
		if ( !state.Def )
		{
			expired.Add( pair.Key );
			continue;
		}

		if ( state.Def->GetTickInterval() > 0.f && now >= state.NextTickAt )
		{
			state.Def->OnTick( owner, state );
			state.NextTickAt = now + state.Def->GetTickInterval();
		}

		if ( now >= state.ExpiresAt )
		{
			expired.Add( pair.Key );
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
			Active_.Remove( key );
		}
	}
}

void UStatusEffectTracker::ApplyStatus( UStatusEffectDef* def )
{
	if ( !def || def->StatusTag.IsNone() )
	{
		return;
	}

	const float now = GetWorld()->GetTimeSeconds();

	if ( FActiveStatus* existing = Active_.Find( def->StatusTag ) )
	{
		existing->ExpiresAt = now + def->Duration;
		if ( def->GetTickInterval() > 0.f )
		{
			existing->NextTickAt = now + def->GetTickInterval();
		}
		return;
	}

	FActiveStatus state;
	state.Def = def;
	state.ExpiresAt = now + def->Duration;
	state.NextTickAt = def->GetTickInterval() > 0.f ? now + def->GetTickInterval() : MAX_flt;

	def->OnApply( GetOwner(), state );

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
	}
	Active_.Empty();
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
