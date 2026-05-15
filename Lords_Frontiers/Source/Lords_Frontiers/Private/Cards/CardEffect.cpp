#include "Cards/CardEffect.h"

#include "Cards/Visuals/CardVFXAsset.h"

const FCardVisualConfig& UCardEffect::GetVisualConfig() const
{
	if ( VFX )
	{
		return VFX->Config;
	}
	return VisualConfig;
}
