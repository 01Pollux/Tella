#pragma once

#include <Windows.h>
#include "../Helpers/D3DX.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"


void ImGui_Impl_Init(void* hWnd, IDirect3DDevice9* pDevice);
