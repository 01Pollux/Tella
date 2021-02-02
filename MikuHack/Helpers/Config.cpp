#include "Config.h"
#include <string>
#include <sstream>

#include "../Source/Main.h"


bool MikuConfig::LoadSettings(const char* path)
{
	Json::Value settings;
	std::ifstream configDoc(path, std::ifstream::binary);
	configDoc >> settings;

	for (auto& entry : AutoList<MenuPanel>::List())
	{
		entry->JsonCallback(settings, true);
	}

	return true;
}

bool MikuConfig::SaveSettings(const char* path)
{
	Json::Value settings;
	Json::StyledWriter styledWriter;

	for (auto& entry : AutoList<MenuPanel>::List())
	{
		entry->JsonCallback(settings, false);
	}

	std::ofstream(path) << styledWriter.write(settings);
	return true;
}