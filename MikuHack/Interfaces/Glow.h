#pragma once

#include <tier1/utlvector.h>
#include <stdint.h>
#include "CBaseEntity.h"


class IGlowManager
{
public:
	int Register(const IBaseHandle& pEnt, const Vector& color, float alpha)
	{
		int index;
		if (first_free_slot == ObjectDefinition::END_OF_FREE_LIST)
		{
			index = objects.AddToTail();
		}
		else
		{
			index = first_free_slot;
			first_free_slot = objects[index].next_free_slot;
		}

		objects[index].entity = pEnt;
		objects[index].color = color;
		objects[index].alpha = alpha;
		objects[index].render_when_occluded = true;
		objects[index].render_when_unoccluded = false;
		objects[index].split_screen_slot = -1;
		objects[index].next_free_slot = ObjectDefinition::ENTRY_IN_USE;

		return index;
	}

	void Unregister(int index)
	{
		objects[index].next_free_slot = first_free_slot;
		objects[index].entity = NULL;
		first_free_slot = index;
	}

	struct ObjectDefinition
	{
		IBaseHandle entity;
		Vector		color;
		float		alpha;

		bool		render_when_occluded;
		bool		render_when_unoccluded;
		int			split_screen_slot;

		int			next_free_slot;

		static const int END_OF_FREE_LIST = -1;
		static const int ENTRY_IN_USE = -2;
	};
	static_assert(sizeof(ObjectDefinition) == 0x20, "sizeof(IGlowManager::ObjectDefinition) mismatch");

	CUtlVector<ObjectDefinition> objects;
	int first_free_slot;
};
extern IGlowManager* GlowManager;

class IGlowObject
{
public:
	IGlowObject(IClientShared* pEnt = NULL, const Color& color = { 255, 255, 255, 255 })
	{
		hndl = GlowManager->Register(pEnt ? pEnt->GetRefEHandle():INVALID_EHANDLE_INDEX, 
							{ float(color.r()) / 255, float(color.g()) / 255, float(color.b()) / 255 }, float(color.a()) / 255);
	}

	~IGlowObject()
	{
		GlowManager->Unregister(hndl);
	}

	void SetEntity(IClientShared* pEnt)
	{
		auto& objects = GlowManager->objects[hndl];
		objects.entity = pEnt ? pEnt->GetRefEHandle() : INVALID_EHANDLE_INDEX;
	}
	
	void SetColor(const Color& color)
	{
		auto& objects = GlowManager->objects[hndl];
		objects.color = { float(color.r()) / 255, float(color.g()) / 255, float(color.b()) / 255 };
		objects.alpha = float(color.a()) / 255;
	}

	int hndl;
};

