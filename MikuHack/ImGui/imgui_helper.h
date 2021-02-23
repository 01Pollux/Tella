#pragma once

#include <Windows.h>
#include "../Helpers/D3DX.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"

#include <array>


void ImGui_Impl_Init(void* hWnd, IDirect3DDevice9* pDevice);

namespace ImGui
{
    template<typename Type>
    void ColorEdit4_2(const char* name, std::array<Type, 4>& color, ImGuiColorEditFlags flags = 0)
    {
        static float actual_color[4];
        for (char8_t i = 0; i < 4; i++)
            actual_color[i] = static_cast<float>(color[i]) / 255;
        
        if (ImGui::ColorEdit4(name, actual_color, flags))
            for (char8_t i = 0; i < 4; i++)
                color[i] = static_cast<char8_t>(actual_color[i] * 255);
    }


    void DrawHelp(const char* text);
}