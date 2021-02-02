#pragma once
#include <Windows.h>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "json.h"
#include "../ImGui/imgui_helper.h"

namespace MikuConfig
{
	bool LoadSettings(const char* path);
	bool SaveSettings(const char* path);
}
