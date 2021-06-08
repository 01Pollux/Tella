#include "ConVar.hpp"
#include "GlobalHook/listener.hpp"

class ConCommandBase_Loader
{
public:
	ConCommandBase_Loader()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				ConCommandBase::Register();
			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			[this](M0EventData*)
			{
				ConCommandBase::Unregister();
			},
			EVENT_NULL_NAME
		);
	}
} static concommand_load;