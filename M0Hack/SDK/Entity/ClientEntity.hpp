#pragma once

#include "ClientEntity_Internal.hpp"


class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	// Delete yourself.
	virtual void			Release() = 0;

	// Network origin + angles
	virtual const Vector&	GetAbsOrigin() const = 0;
	virtual const QAngle&	GetAbsAngles() const = 0;

	virtual CMouthInfo*		GetMouth() = 0;

	// Retrieve sound spatialization info for the specified sound on this entity
	// Return false to indicate sound is not audible
	virtual bool			GetSoundSpatialization(SpatializationInfo& info) = 0;
};