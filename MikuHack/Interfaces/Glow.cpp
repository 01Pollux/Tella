#include "Glow.h"
#include "../Helpers/Library.h"

IGlowManager* GlowManager;
auto GlowContainer = new std::unordered_map<IClientShared*, IGlowObject*>;

IGlowObject* IClientShared::AllocateVirtualGlowObject(const Color& color, bool create)
{
	auto iter = GlowContainer->find(this);
	if (iter == GlowContainer->end())
	{
		if (create)
		{
			IGlowObject* obj = new IGlowObject(this, color);
			GlowContainer->insert(std::make_pair(this, obj));
			return obj;
		}
		else return nullptr;
	}
	return iter->second;
}

void IClientShared::DestroyVirtualGlowObject()
{
	auto iter = GlowContainer->find(this);
	if (iter != GlowContainer->end())
	{
		delete iter->second;
		GlowContainer->erase(iter);
	}
}

IGlowObject* IClientShared::QueryVirtualGlowObject()
{
	auto iter = GlowContainer->find(this);
	return iter != GlowContainer->end() ? iter->second : nullptr;
}


#include "../Interfaces/HatCommand.h"
#include "../Helpers/DrawTools.h"

static bool toggle = true;
HAT_COMMAND(glow_toggle_self, "Toggle glow effect (local player)")
{
	using namespace DrawTools::ColorTools;
	ITFPlayer* pMe = pLocalPlayer;

	if (toggle)
	{
		IGlowObject* pObj = pMe->AllocateVirtualGlowObject(FromArray(Cyan<char8_t>));
		REPLY_TO_TARGET(void, "GlowObject: %p create\n", pObj);
	}
	else {
		IGlowObject* pObj = pMe->AllocateVirtualGlowObject(FromArray(Cyan<char8_t>), false);
		REPLY_TO_TARGET(void, "GlowObject: %p destroyed\n", pObj);

		pMe->DestroyVirtualGlowObject();
		pObj = pMe->AllocateVirtualGlowObject(FromArray(Cyan<char8_t>), false);
	}

	toggle = !toggle;
}
