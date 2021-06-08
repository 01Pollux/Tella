#pragma once


#include "winmain.hpp"

#include "imgui.h"
#include "imgui_impl_dx9.h"

#include "Helper/Config.hpp"
#include "Helper/Timer.hpp"

void ImGui_Impl_Init(void* hWnd, IDirect3DDevice9* pDevice);

namespace ImGui
{
    template<typename Type>
    inline bool ColorEdit3(const char* name, std::array<Type, 3>& color, ImGuiColorEditFlags flags = 0)
    {
        float actual_color[3]{
            static_cast<float>(color[0]) / 255,
            static_cast<float>(color[1]) / 255,
            static_cast<float>(color[2]) / 255,
        };
        
        if (ImGui::ColorEdit4(name, actual_color, flags))
        {
            for (Type i = 0; i < 3; i++)
                color[i] = static_cast<Type>(actual_color[i] * 255);
            return true;
        }
        return false;
    } 
    
    template<typename Type>
    inline bool ColorEdit4(const char* name, std::array<Type, 4>& color, ImGuiColorEditFlags flags = 0)
    {
        float actual_color[4]{
            static_cast<float>(color[0]) / 255,
            static_cast<float>(color[1]) / 255,
            static_cast<float>(color[2]) / 255,
            static_cast<float>(color[3]) / 255
        };

        if (ImGui::ColorEdit4(name, actual_color, flags))
        {
            for (Type i = 0; i < 4; i++)
                color[i] = static_cast<Type>(actual_color[i] * 255);
            return true;
        }
        return false;
    }

    template<typename Type>
    inline bool ColorButton(const char* name, std::array<Type, 4>& color, ImGuiColorEditFlags flags = 0, ImVec2 size = { 0, 0 })
    {
        ImVec4 actual_color
        {
            static_cast<float>(color[0]) / 255,
            static_cast<float>(color[1]) / 255,
            static_cast<float>(color[2]) / 255,
            static_cast<float>(color[3]) / 255
        };
        
        if (ImGui::ColorButton(name, actual_color, flags, size))
        {
            for (Type i = 0; i < 4; i++)
                color[i] = static_cast<Type>(actual_color[i] * 255);
            return true;
        }
        return false;
    }

    void Help(const char* text);

    inline void Help(const M0Config::M0VarStorage& var)
    {
        if (var.has_description())
            Help(var.get_description());
    }

    inline void SameLineHelp(const char* text)
    {
        ImGui::SameLine();
        Help(text);
    }

    inline void SameLineHelp(const M0Config::M0VarStorage& var)
    {
        if (var.has_description())
        {
            ImGui::SameLine();
            Help(var.get_description());
        }
    }

    template<class AutoTimer>
    inline bool DragTime(
        const char* label, 
        AutoTimer* timer, 
        float v_speed = 1.0f, 
        float v_min = 0.0f, 
        float v_max = 0.0f, 
        const char* format = "%.3f", 
        ImGuiSliderFlags flags = ImGuiSliderFlags_None
    )
    {
        float v = timer->get_time();
        if (DragFloat(label, &v, v_speed, v_min, v_max, format, flags))
        {
            timer->set_time(v);
            return true;
        }
        return false;
    }

    void InputText(
        const char* label, 
        std::string* buffer, 
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_None,
        ImGuiInputTextCallback callback = nullptr, 
        void* extra_data = nullptr
    );
    void InputTextMultiline(
        const char* label, 
        std::string* buffer,
        const ImVec2& size,
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_None,
        ImGuiInputTextCallback callback = nullptr,
        void* extra_data = nullptr
    );
    void InputTextWithHint(
        const char* label, 
        const char* hint, 
        std::string* buffer, 
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_None, 
        ImGuiInputTextCallback callback = nullptr, 
        void* extra_data = nullptr
    );
}