
#include "TFHuds.h"

#include "../Helpers/Library.h"
#include "../Helpers/Offsets.h"

ITFHudManager::ITFHudManager()
{
	namespace HUD = Offsets::ClientDLL::HUD;

	uintptr_t** gHUD = reinterpret_cast<uintptr_t**>(Library::clientlib.FindPattern("gHUD"));

	huds = reinterpret_cast<IHudElement**>(gHUD[HUD::Manager::This_To_Vec]);
	count = reinterpret_cast<int>(gHUD[HUD::Manager::This_To_SizeOfVec]);
}