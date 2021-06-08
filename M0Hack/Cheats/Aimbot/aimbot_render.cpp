#include "aimbot.hpp"

void IAimbotHack::OnRender()
{
    if (ImGui::BeginTabItem("Aimbot"))
    {
        if (ImGui::CollapsingHeader("Debug"))
        {
            auto aimdebug = &Vars.Debug;

            ImGui::Checkbox("Draw Hitboxes", aimdebug->DrawHitbox.data());
            ImGui::SameLineHelp(aimdebug->DrawHitbox);

            ImGui::Checkbox("Draw Box", aimdebug->DrawBox.data());
            ImGui::SameLineHelp(aimdebug->DrawBox);

            ImGui::Checkbox("Draw Line", aimdebug->DrawLine.data());
            ImGui::SameLineHelp(aimdebug->DrawLine);

            ImGui::DragTime("Time", aimdebug->RepeatTimer.data(), 0.5f, 0.f, 15.f);
            ImGui::SameLineHelp(aimdebug->RepeatTimer);
        }
        
        ImGui::Checkbox("Enable", Vars.Enable.data());

        {
            ImGui::Checkbox("Player Only", Vars.PlayerOnly.data());
            ImGui::SameLineHelp(Vars.PlayerOnly);

            ImGui::SameLine();
            ImGui::Checkbox("Allow Buildings", Vars.AimForBuildings.data());
            ImGui::SameLineHelp(Vars.AimForBuildings);

            ImGui::SameLine();
            ImGui::Checkbox("Allow Projectiles", Vars.AimForProjectiles.data());
            ImGui::SameLineHelp(Vars.AimForBuildings);
        }

        ImGui::Checkbox("Auto Aim", Vars.AutoAim.data());
        ImGui::SameLineHelp(Vars.AutoAim);
        ImGui::SameLine();

        ImGui::Checkbox("Auto Shoot", Vars.AutoShoot.data());
        ImGui::SameLineHelp(Vars.AutoShoot);

        ImGui::Checkbox("Zoom Only", Vars.OnlyZoom.data());
        ImGui::SameLineHelp(Vars.OnlyZoom);

        ImGui::Checkbox("Target Lock", Vars.TargetLock.Enable.data());
        ImGui::SameLineHelp(Vars.TargetLock.Enable);

        ImGui::Checkbox("Key Lock", Vars.KeyLock.data());
        ImGui::SameLineHelp(Vars.KeyLock);

        ImGui::Checkbox("Toggle", Vars.CanToggle.data());
        ImGui::SameLineHelp(Vars.CanToggle);

        ImGui::InputInt("Aim VM Key", Vars.AimKey.data(), 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::SameLineHelp(Vars.AimKey);

        ImGui::SliderInt("Miss Chance", Vars.MissChance.data(), 0, 100);
        ImGui::SameLineHelp(Vars.MissChance);

        ImGui::SliderInt("Max Range", Vars.MaxRange.data(), 0, 4096);
        ImGui::SameLineHelp(Vars.MaxRange);


        {
            ImGui::Combo(
                "Target Priority",
                reinterpret_cast<int*>(Vars.TargetPriority.data()),
                    "FOV\0"
                    "FOV And Distance\0"
                    "Highest Score\0"
                    "Lowest Score\0"
                    "Highest Health\0"
                    "Lowest Health\0"
                    "Closest\0"
                    "Furthest\0\0"
            );

            ImGui::Combo(
                "Hitbox Priority", 
                reinterpret_cast<int*>(Vars.HitboxPriority.data()),
                    "Head\0"
                    "Nearest\0"
                    "Smart\0\0"
            );

            ImGui::InputFloat("FOV", Vars.FOV.data());
            ImGui::SameLineHelp(Vars.FOV);

            ImGui::BeginGroup();
            {
                auto* Smooth = &Vars.Smooth;

                ImGui::InputFloat("Default", Smooth->Standard.data());
                ImGui::SameLineHelp(Smooth->Standard);

                ImGui::DragFloat("Sniper", Smooth->Sniper.data(), 0.05f, 0.f, 1.0f, "%.2f");
                ImGui::SameLineHelp(Smooth->Sniper);

                ImGui::DragFloat("Spy", Smooth->Spy.data(), 0.05f, 0.f, 1.0f);
                ImGui::SameLineHelp(Smooth->Spy);

                ImGui::DragFloat("Soldier", Smooth->Soldier.data(), 0.05f, 0.f, 1.0f);
                ImGui::SameLineHelp(Smooth->Soldier);
            }
            ImGui::EndGroup();
        }

        {
            ImGui::Checkbox("Bypass Cloaks", Vars.BypassCloak.data());
            ImGui::SameLineHelp(Vars.BypassCloak);

            ImGui::SameLine();
            ImGui::Checkbox("Bypass Conds", Vars.BypassBadConds.data());
            ImGui::SameLineHelp(Vars.BypassBadConds);

            ImGui::SameLine();
            ImGui::Checkbox("Only Medics", Vars.MedicOnly.data());
            ImGui::SameLineHelp(Vars.MedicOnly);
        }

        ImGui::BeginGroup();
        if (ImGui::BeginChild("Effects", { 0, 110.f }, true))
        {
            auto GlowInfo = &Vars.GlowInfo;
            auto Particle = &Vars.ParticleInfo;

            if (ImGui::RadioButton("None", !GlowInfo->Enable && !Particle->Enable))
            {
                GlowInfo->Enable = false;
                Particle->Enable = false;
                GlowInfo->Handler_.invalidate();
            }
            ImGui::SameLineHelp("No Particle & No Glow");

            {
                if (ImGui::RadioButton("Glow", GlowInfo->Enable))
                {
                    GlowInfo->Enable = true;
                    Particle->Enable = false;
                }

                ImGui::SameLineHelp(GlowInfo->Enable);

                {
                    ImGui::ColorEdit4("Glow Color", GlowInfo->Color.get(), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8);
                    ImGui::SameLineHelp(GlowInfo->Color);
                }
            }
            
            {
                if (ImGui::RadioButton("Particles", Particle->Enable))
                {
                    Particle->Enable = true;
                    GlowInfo->Enable = false;
                    GlowInfo->Handler_.invalidate();
                }
                ImGui::SameLineHelp(Particle->Enable);

                ImGui::InputText("##PARTICLE NAME", Particle->String.data());
                ImGui::SameLineHelp(Particle->String);
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();

        ImGui::EndTabItem();
    }
}