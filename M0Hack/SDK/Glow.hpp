#pragma once

#include <memory>
#include "UtlVector.hpp"
#include "Entity/HandleEntity.hpp"
#include "Helper/Color.hpp"
#include "MathLib/Vector3D.hpp"
#include "Interfaces.hpp"


class IBaseEntityInternal;
class IBaseHandle;

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
			first_free_slot = objects[index].NextFreeSlot;
		}

		objects[index].Entity = pEnt;
		objects[index].Color = color;
		objects[index].Alpha = alpha;
		objects[index].RenderWhenOccluded = true;
		objects[index].RenderWhenUnoccluded = false;
		objects[index].SplitScreenSlot = -1;
		objects[index].NextFreeSlot = ObjectDefinition::ENTRY_IN_USE;

		return index;
	}

	void Unregister(int index) noexcept
	{
		objects[index].NextFreeSlot = first_free_slot;
		objects[index].Entity = nullptr;
		first_free_slot = index;
	}

	struct ObjectDefinition
	{
		ObjectDefinition() = default;

		IBaseHandle	Entity;
		Vector		Color;
		float		Alpha;

		bool		RenderWhenOccluded;
		bool		RenderWhenUnoccluded;
		int			SplitScreenSlot;

		int			NextFreeSlot;

		static const int END_OF_FREE_LIST = -1;
		static const int ENTRY_IN_USE = -2;
	};

	static_assert(sizeof(ObjectDefinition) == 0x20, "sizeof(IGlowManager::ObjectDefinition) mismatch");

	ValveUtlVector<ObjectDefinition> objects;
	int first_free_slot;
};

M0_INTERFACE;
extern IGlowManager* GlowManager;
M0_END;

class IGlowObject
{
public:
	explicit IGlowObject(const IBaseEntityInternal* pEnt = nullptr, const color::u8rgba& color = color::names::white);

	~IGlowObject() noexcept
	{
		Interfaces::GlowManager->Unregister(HandleIdx);
	}

	void SetEntity(const IBaseEntityInternal* pEnt);
	void SetColor(const color::u8rgba& color);

	IGlowObject(const IGlowObject&) = delete;
	IGlowObject& operator=(const IGlowObject&) = delete;
	IGlowObject(IGlowObject&&) = delete;
	IGlowObject& operator=(IGlowObject&&) = delete;
	
private:
	int HandleIdx;
};

using IUniqueGlowObject = std::unique_ptr<IGlowObject>;