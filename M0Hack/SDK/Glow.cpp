#include <unordered_map>
#include "Glow.hpp"
#include "Entity/BaseEntity.hpp"
#include "Library/Lib.hpp"


namespace GlobalGlow
{
	static std::unordered_map<IBaseEntityInternal*, IGlowObject*> glow_container;
}

const auto& GlowContainer = &GlobalGlow::glow_container;

IGlowObject::IGlowObject(const IBaseEntityInternal* pEnt, const color::u8rgba& color)
{
	HandleIdx = Interfaces::GlowManager->Register(
		pEnt ? pEnt->GetRefEHandle() : INVALID_ENTITY_HANDLE,
		{ static_cast<float>(color.r) / 255, static_cast<float>(color.g) / 255, static_cast<float>(color.b) / 255 },
		static_cast<float>(color.a) / 255
	);
}

void IGlowObject::SetEntity(const IBaseEntityInternal* pEnt)
{
	auto& objects = Interfaces::GlowManager->objects[HandleIdx];
	objects.Entity = pEnt ? pEnt->GetRefEHandle() : INVALID_ENTITY_HANDLE;
}

void IGlowObject::SetColor(const color::u8rgba& color)
{
	auto& objects = Interfaces::GlowManager->objects[HandleIdx];
	objects.Color = { static_cast<float>(color.r) / 255, static_cast<float>(color.g) / 255, static_cast<float>(color.b) / 255 };
	objects.Alpha = static_cast<float>(color.a) / 255;
}

IGlowObject* IBaseEntityInternal::AllocateVirtualGlowObject(const color::u8rgba& color, bool create_if_deleted)
{
	const auto iter = GlowContainer->find(this);
	if (iter == GlowContainer->end())
	{
		if (create_if_deleted)
		{
			IGlowObject* obj = new IGlowObject(this, color);
			GlowContainer->insert(std::make_pair(this, obj));
			return obj;
		}
		else return nullptr;
	}
	return iter->second;
}

void IBaseEntityInternal::DestroyVirtualGlowObject()
{
	const auto iter = GlowContainer->find(this);
	if (iter != GlowContainer->end())
	{
		delete iter->second;
		GlowContainer->erase(iter);
	}
}

IGlowObject* IBaseEntityInternal::QueryVirtualGlowObject()
{
	const auto iter = GlowContainer->find(this);
	return iter != GlowContainer->end() ? iter->second : nullptr;
}