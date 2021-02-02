
#include "Aimbot.h"
#include "../Interfaces/IVEClientTrace.h"
#include "../Helpers/Commons.h"
#include "../Interfaces/IClientMode.h"
#include "../Helpers/DrawTools.h"
#include "../Helpers/VTable.h"

#include "Hack/Effects.h"
#include "../Interfaces/Glow.h"

#include <thread>
#include <regex>
#include <set>


static IBaseHandle pHndl;
AutoVar<ITFPlayer*> pLastEnt{ "Aimbot::CurEntity" };
static bool m_bFoundTarget = false;
static bool m_bCanSnap = false;

//
enum TargetPriority_t
{
    HIGHEST_SCORE,
    CLOSEST_FOV,
    LOWEST_HP,
    HIGHEST_HP,
    CLOSEST_ENT,
    FURTHEST_AWAY,
};

enum HitBoxPriority_t
{
    HB_HEAD,
    HB_CLOSEST,
    HB_CUSTOM
};

struct _AimDataInfo
{
    AutoBool bIsEnabled{ "Aimbot::Enabled",         true };
    AutoBool bAutoAim{ "Aimbot::AutoAim",           false };
    AutoBool bZoomedOnly{ "Aimbot::ZoomOnly",       true };
    AutoBool bAimLock{ "Aimbot::AimLock",           true };
    AutoBool bKeyLock{ "Aimbot::KeyLock",           false };
    AutoBool bUseBackTrack{ "Aimbot::BackTrack",    false };

    AutoBool bDrawParticles{ "Aimbot::DrawParticles", true };
    AutoBool bUseGlow{ "Aimbot::UseGlow",           false };
    AutoColor cAimGlow{ "Aimbot::GlowColor"};

    AutoInt iAimKey{ "Aimbot::AimKey",          VK_DELETE };
    AutoInt iRandomMiss{ "Aimbot::RandomMiss",  21 };

    AutoInt ePriority{ "Aimbot::ePriority",     CLOSEST_FOV };
    AutoInt eHitboxPos{ "Aimbot::eHitboxPos",   HB_CUSTOM };

    AutoFloat flAimFOV{ "Aimbot::AimFOV",               45.f };
    AutoFloat flSmoothFactor{ "Aimbot::SmoothFactor",   29.f };

    AutoString sAimParticle{ "Aimbot::AimParticle", "soldierbuff_blue_buffed" };

    IGlowObject* pGlowEnt = nullptr;

    bool bShouldAim = true;

    _AimDataInfo()
    {
        cAimGlow.Set(DrawTools::ColorTools::Cyan<char8_t>);
    }

    inline void ToggleState(bool state)
    {
        this->bShouldAim = state;
    }

    inline bool CanAim()
    {
        return this->bShouldAim;
    }

    inline void SetKey(int i)
    {
        switch (i)
        {
        case 0:
            iAimKey = static_cast<int>('Q');
            break;
        case 1:
            iAimKey = VK_END;
            break;
        case 2:
            iAimKey = VK_F9;
            break;
        case 3:
            iAimKey = VK_DELETE;
            break;
        }
    }

    inline int KeyToPos()
    {
        switch (iAimKey)
        {
        case 'Q':
            return 0;
        case VK_END:
            return 1;
        case VK_F9:
            return 2;
        case VK_DELETE:
            return 3;
        }
        return 0;
    }

    inline void AllocateGlowOnce()
    {
        if (!pGlowEnt)
            pGlowEnt = new IGlowObject;
    }

    inline void DestroyGlowOnce()
    {
        if (pGlowEnt)
        {
            delete pGlowEnt;
            pGlowEnt = nullptr;
        }
    }
} aimbot_data;

static std::map<ITFPlayer*, Timer> m_MapTimer;

static bool FindBestTarget();

static bool IsProjectileBased(ClassID clsID)
{
    switch (clsID)
    {
    case ClassID_CTFRocketLauncher_DirectHit:
    case ClassID_CTFRocketLauncher:
    case ClassID_CTFGrenadeLauncher:
    case ClassID_CTFPipebombLauncher:
    case ClassID_CTFCompoundBow:
    case ClassID_CTFBat_Wood:
    case ClassID_CTFBat_Giftwrap:
    case ClassID_CTFFlareGun:
    case ClassID_CTFFlareGun_Revenge:
    case ClassID_CTFSyringeGun:
    case ClassID_CTFCrossbow:
    case ClassID_CTFShotgunBuildingRescue:
    case ClassID_CTFDRGPomson:
    case ClassID_CTFWeaponFlameBall:
    case ClassID_CTFRaygun:
    case ClassID_CTFGrapplingHook:
    case ClassID_CTFJar:
    case ClassID_CTFJarGas:
    case ClassID_CTFJarMilk:
        return true;
    }
    return false;
}

static bool IsSniperRifle(ClassID clsID)
{
    switch (clsID)
    {
    case ClassID_CTFSniperRifle:
    case ClassID_CTFSniperRifleClassic:
    case ClassID_CTFSniperRifleDecap:
        return true;
    }
    return false;
}

inline void _InvalidateAimLock()
{
    m_bFoundTarget = false;
    pLastEnt = nullptr;
    aimbot_data.pGlowEnt->SetEntity(nullptr);
}

static bool IsValidPlayer(ITFPlayer*);
static bool GetBestPlayerPos(ITFPlayer*, Vector*);
static inline void SmoothAngle(QAngle&, float);


struct KillParticleInfo
{
    CTFParticle* pParticle;
    CTFParticleFactory* pFactory;
    ITFPlayer* pOwner;
};


class _EntListener : public IClientEntityListener
{
    void OnEntityDeleted(IClientShared* pEnt) override
    {
        auto i = m_MapTimer.find(static_cast<ITFPlayer*>(pEnt));
        if (i != m_MapTimer.end())
            Timer::DeleteFuture(&i->second, true);
    }
} static _ent_listener;


static void Timer_KillParticle(void* data)
{
    if (KillParticleInfo* pInfo = reinterpret_cast<KillParticleInfo*>(data))
    {
        pInfo->pFactory->StopEmission(pInfo->pParticle);
        m_MapTimer.erase(pInfo->pOwner);
        delete pInfo;
    }
}



HookRes AimbotMenu::OnCreateMove(bool& ret)
{
    if (pLocalPlayer->GetLifeState() != LIFE_STATE::ALIVE)
        return HookRes::Continue;

    if (!aimbot_data.bIsEnabled)
        return HookRes::Continue;

    static std::atomic<bool> _invalidated = false;
    if (!(Globals::m_pUserCmd->buttons & IN_ATTACK) && !aimbot_data.bAutoAim)
    {
        if (!_invalidated)
        {
            _InvalidateAimLock();
            _invalidated = true;
        }
        return HookRes::Continue;
    }

    //Update ClassID && Key
    {
        static Timer timer_update_key;
        bool bKeyPress = GetAsyncKeyState(aimbot_data.iAimKey);

        if (aimbot_data.bKeyLock)         aimbot_data.ToggleState(bKeyPress);
        else if (bKeyPress && timer_update_key.has_elapsed(500))
        {
            aimbot_data.ToggleState(!aimbot_data.CanAim());
            timer_update_key.update();
        }
    }

    if (!aimbot_data.CanAim())
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }

    // Update sniper rifle
    IBaseObject* pWep = pLocalWeapon;
    ClassID clsID = pWep ? static_cast<ClassID>(pWep->GetClientClass()->m_ClassID) : ClassID_CTFPlayer;
    static Timer timer_snap_track;
    {
        if (aimbot_data.bZoomedOnly && IsSniperRifle(clsID))
            if (pLocalPlayer->InCond(ETFCond::TF_COND_ZOOMED))
                timer_snap_track.update();
    }

    if (aimbot_data.iRandomMiss && (Random::Int(0, 100) < aimbot_data.iRandomMiss))
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }

    if (!FindBestTarget())
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }
    
    Vector vecBestPos;
    if (!GetBestPlayerPos(pLastEnt, &vecBestPos))
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }

    _invalidated = false;
    ret = true;

    QAngle angRes;
    angRes = GetAimAngle(vecBestPos);

    if (aimbot_data.flSmoothFactor && timer_snap_track.has_elapsed(300))
        SmoothAngle(angRes, aimbot_data.flSmoothFactor);
    else ret = false;

    Globals::m_pUserCmd->viewangles = angRes;

    {
        static Timer update_line;
        if (MIKUDebug::m_bDebugging && update_line.trigger_if_elapsed(1800))
            debugoverlay->AddLineOverlay(pLocalPlayer->LocalEyePosition(), vecBestPos, 255, 255, 0, false, 2.0f);
    }

    if (aimbot_data.bUseGlow)
    {
        aimbot_data.pGlowEnt->SetEntity(pLastEnt);
        aimbot_data.pGlowEnt->SetColor(DrawTools::ColorTools::FromArray(aimbot_data.cAimGlow.get()));
    }
    else if (aimbot_data.bDrawParticles)
    {
        // Display Particles
        static Timer timer_update_particles;
        if (timer_update_particles.trigger_if_elapsed(130))
        {
            auto i = m_MapTimer.find(*pLastEnt);

            if (i != m_MapTimer.end())
                Timer::RollBack(&i->second);
            else {
                auto factory = CTFParticleFactory::ParticleProp(pLastEnt);

                KillParticleInfo* pInfo = new KillParticleInfo{
                            factory->Create((*aimbot_data.sAimParticle).c_str(), PATTACH_ABSORIGIN_FOLLOW),
                            factory,
                            *pLastEnt
                };

                Timer t = Timer::CreateFuture(0.15, TIMER_EXECUTE_ON_MAP_END, Timer_KillParticle, pInfo);
                m_MapTimer.insert(std::make_pair(*pLastEnt, t));
            }
        }
    }

    return HookRes::Continue;
}

AimbotMenu::AimbotMenu()
{
    using namespace IGlobalEvent;
    using std::bind;

    LoadDLL::Hook::Register(
        []() -> HookRes
        { 
            if (!(BAD_LOCAL()))
                aimbot_data.AllocateGlowOnce();

            return HookRes::Continue;
        });
    UnloadDLL::Hook::Register([]() -> HookRes { aimbot_data.DestroyGlowOnce(); return HookRes::Continue; });

    LevelInit::Hook::Register([]() -> HookRes { aimbot_data.AllocateGlowOnce(); return HookRes::Continue; });
    LevelShutdown::Hook::Register([]() -> HookRes { aimbot_data.DestroyGlowOnce(); return HookRes::Continue; });

    CreateMove::Hook::Register(bind(&AimbotMenu::OnCreateMove, this, std::placeholders::_1));
}


static bool IsValidPlayer(ITFPlayer* pPlayer)
{
    if (!pPlayer || pPlayer->IsDormant())
        return false;

    if (pPlayer->GetLifeState() != LIFE_STATE::ALIVE)
        return false;

    ITFPlayer* pMe = pLocalPlayer;

    if (pPlayer == pMe || pPlayer->GetTeam() == pMe->GetTeam())
        return false;

    IBaseObject* pActiveWep = pLocalWeapon;

    static Vector vecPos; vecPos = pPlayer->GetAbsOrigin();

    // if we have active melee
    {
        if (pActiveWep->GetWeaponSlot() == 2)
            if (vecPos.DistTo(pLocalPlayer->GetAbsOrigin()) > pActiveWep->Melee_GetSwingRange())
                return false;
    }

    //filter conds
    {
        static const std::set<ETFCond> _BadConds = {
            TF_COND_STEALTHED_USER_BUFF,
            TF_COND_INVULNERABLE,
            TF_COND_DEFENSEBUFF,
            TF_COND_DEFENSEBUFF_HIGH,
            TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
            TF_COND_FEIGN_DEATH,
            TF_COND_DISGUISED,
        };

        for (auto cond : _BadConds)
        {
            if (pPlayer->InCond(cond))
                return false;
        }
    }

    return true;
}


bool FindBestTarget()
{
    if (aimbot_data.bAimLock && m_bFoundTarget)
        if (pHndl.IsValid() && IsValidPlayer(pLastEnt))
            return true;
        else m_bFoundTarget = false;

    float flCur = 0.0, flBest = 99999.9f;
    ITFPlayer* pBest = nullptr, * pCur;
    ITFPlayer* pMe = pLocalPlayer;
    Vector pEyeVec = pMe->LocalEyePosition();

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        pCur = reinterpret_cast<ITFPlayer*>(GetClientEntityW(i));

        if (IsValidPlayer(pCur))
        {
            switch (aimbot_data.ePriority)
            {
            case TargetPriority_t::CLOSEST_ENT:
                flCur = pCur->GetAbsOrigin().DistTo(pMe->GetAbsOrigin());
                break;

            case TargetPriority_t::FURTHEST_AWAY:
                flCur = -pCur->GetAbsOrigin().DistTo(pMe->GetAbsOrigin());
                break;

            case TargetPriority_t::CLOSEST_FOV:
                flCur = GetLocalFOV(pCur->EyePosition());
                if (flCur > aimbot_data.flAimFOV)
                    continue;
                break;

            case TargetPriority_t::LOWEST_HP:
                flCur = float(*pCur->GetEntProp<int>("m_iHealth"));
                break;

            case TargetPriority_t::HIGHEST_HP:
                flCur = -float(*pCur->GetEntProp<int>("m_iHealth"));
                break;

            case TargetPriority_t::HIGHEST_SCORE:
                flCur = -float(ctfresource.GetEntProp<int>(pCur, "m_iScore"));
                break;
            }

            if (flBest > flCur)
            {
                flBest = flCur;
                pBest = pCur;
            }
        }
    }
    m_bFoundTarget = pBest != nullptr;
    pLastEnt = pBest;

    if (!m_bFoundTarget)
        return false;

    pHndl = pBest->GetRefEHandle();

    return true;
}


static bool GetClosestHitbox(IClientShared* pEnt, Vector*& vec, IBoneCache* cache)
{
    const model_t* mdl = pEnt->GetModel();
    if (!mdl)
        return false;

    studiohdr_t* pStudioHDR = modelinfo->GetStudiomodel(mdl);
    if (!pStudioHDR)
        return false;

    float nearest = 99999999.f, cur;

    mstudiohitboxset_t* pStudioBoxSet = pStudioHDR->pHitboxSet(pEnt->HitboxSet());

    Vector hitbox_pos, vecFinal = vec3_origin;
    ITFPlayer* pMe = pLocalPlayer; 
    Vector eye_pos = pMe->LocalEyePosition();

    for (int i = 0; i < pStudioBoxSet->numhitboxes; i++)
    {
        if (!pEnt->GetHitbox(i, cache))
            continue;

        hitbox_pos = cache->center;
        if (!Trace::VectorIsVisible(eye_pos, hitbox_pos, pMe))
            continue;

        cur = GetLocalFOV(hitbox_pos);
        if (cur < nearest && cur <= aimbot_data.flAimFOV)
        {
            vecFinal = hitbox_pos;
            nearest = cur;
        }
    }

    *vec = vecFinal;
    return vecFinal != vec3_origin;
}

bool GetBestPlayerPos(ITFPlayer* pEnt, Vector* vecFinal)
{
    IBaseObject* pActiveWeapon = pLocalWeapon;
    ITFPlayer* pMe = pLocalPlayer;

    std::unique_ptr<IBoneCache> cache = std::make_unique<IBoneCache>();

    Vector vecRes;
    switch (aimbot_data.eHitboxPos)
    {
    case HitBoxPriority_t::HB_HEAD:
    {
        if (pEnt->GetHitbox(hitbox_t::head, cache.get()))
        {
            vecRes = cache->center;

            if (Trace::VectorIsVisible(pMe->LocalEyePosition(), vecRes, pMe))
            {
                *vecFinal = vecRes;
                return true;
            }
        }
    }
    [[fallthrough]];
    case HitBoxPriority_t::HB_CUSTOM:
    {
        bool m_bReadyToShoot = false;

        ClassID clsId = static_cast<ClassID>(pActiveWeapon->GetClientClass()->m_ClassID);
        switch (clsId)
        {
        case ClassID_CTFCompoundBow:
        case ClassID_CTFSniperRifle:
        case ClassID_CTFSniperRifleClassic:
        case ClassID_CTFSniperRifleDecap:
        {
            int hitbox;
            float flDmg = *pActiveWeapon->GetEntProp<float>("m_flChargedDamage");
            if (flDmg >= *(pEnt->GetEntProp<int>("m_iHealth")))
                hitbox = hitbox_t::spine_3;
            else
                hitbox = hitbox_t::head;
            m_bCanSnap = true;

            if (hitbox != hitbox_t::invalid)
            {
                if (pEnt->GetHitbox(hitbox, cache.get()))
                {
                    vecRes = cache->center;
                    if (Trace::VectorIsVisible(pMe->LocalEyePosition(), vecRes, pMe))
                    {
                        *vecFinal = vecRes;
                        return true;
                    }
                }
            }
            break;
        }
        case ClassID::ClassID_CTFBaseRocket:
        {
            if (pEnt->GetHitbox(hitbox_t::hip_L, cache.get()))
            {
                vecRes = cache->center;
                if (Trace::VectorIsVisible(pMe->LocalEyePosition(), vecRes, pMe))
                {
                    *vecFinal = vecRes;
                    return true;
                }
            }
            if (pEnt->GetHitbox(hitbox_t::hip_R, cache.get()))
            {
                vecRes = cache->center;
                if (Trace::VectorIsVisible(pMe->LocalEyePosition(), vecRes, pMe))
                {
                    *vecFinal = vecRes;
                    return true;
                }
            }
        }
        }
    }
    [[fallthrough]];
    case HitBoxPriority_t::HB_CLOSEST:
    {
        return GetClosestHitbox(pEnt, vecFinal, cache.get());
    }
    break;
    }
}

void SmoothAngle(QAngle& angRes, float factor)
{
    QAngle angCur = Globals::m_pUserCmd->viewangles;
    QAngle angDelta = (angCur - angRes) / factor;

    angRes.x = ApproachAngle(angRes.x, angCur.x, angDelta.x);
    angRes.y = ApproachAngle(angRes.y, angCur.y, angDelta.y);

    ClampAngle(angRes);
}


void AimbotMenu::OnRender()
{
    if (ImGui::BeginTabItem("Aimbot"))
    {
        ImGui::Checkbox("Enable", aimbot_data.bIsEnabled.get());

        ImGui::Checkbox("Auto Aim", aimbot_data.bIsEnabled.get());
        ImGui::SameLine(); DrawTools::DrawHelp("Only activate if MOUSE1 is pressed");

        if (aimbot_data.bIsEnabled)
        {
            {
                ImGui::Combo("Priority Types", aimbot_data.ePriority.get(), "Highest Score\0Closest FOV\0Lowest HP\0Highest HP\0Furthest Away\0\0");
                ImGui::SameLine(); DrawTools::DrawHelp("Choose your player priority");
                ImGui::Separator();
            }

            {
                ImGui::Combo("Aim Priority", aimbot_data.eHitboxPos.get(), "Head\0Closest\0Custom\0\0");
                ImGui::SameLine(); DrawTools::DrawHelp("Choose your hitbox priority");
                ImGui::Separator();
            }

            {
                static int m_iCur; m_iCur = aimbot_data.KeyToPos();
                if (ImGui::Combo("Switch Key", &m_iCur, "Q\0END\0F9\0DELETE\0\0"))
                    aimbot_data.SetKey(m_iCur);

                ImGui::SameLine(); DrawTools::DrawHelp("Toggle Aimbot State");
                ImGui::Separator();
            }

            ImGui::Checkbox("Zoomed", aimbot_data.bZoomedOnly.get());
            ImGui::SameLine(); DrawTools::DrawHelp("Sniper: Only activate when you are scoping");
            ImGui::Dummy({ 5, 5 });

            ImGui::Checkbox("Aim-Lock", aimbot_data.bAimLock.get());
            ImGui::SameLine(); DrawTools::DrawHelp("Keep targeting same player (until he is dead / aim reset)");
            ImGui::Dummy({ 5, 5 });

            ImGui::InputInt("Random Miss", aimbot_data.iRandomMiss.get());
            ImGui::SameLine(); DrawTools::DrawHelp("Chance of randomly missing target");
            ImGui::Dummy({ 5, 5 });

            ImGui::InputFloat("FOV", aimbot_data.flAimFOV.get(), 1.0f, 5.0f, "%.0f");
            ImGui::SameLine(); DrawTools::DrawHelp("Field of view to target");
            ImGui::Dummy({ 5, 5 });

            ImGui::Checkbox("Key-Lock", aimbot_data.bKeyLock.get());
            ImGui::SameLine(); DrawTools::DrawHelp("Keep pressing Switch Key for aim to activate");
            ImGui::Dummy({ 5, 5 });

            if (ImGui::BeginChild("Effects", { 0, 110.f }))
            {
                if (ImGui::RadioButton("None", !aimbot_data.bUseGlow && !aimbot_data.bDrawParticles))
                {
                    aimbot_data.bUseGlow = false;
                    aimbot_data.bDrawParticles = false;
                    if (aimbot_data.pGlowEnt)
                        aimbot_data.pGlowEnt->SetEntity(nullptr);
                }
                ImGui::SameLine(); DrawTools::DrawHelp("No Particle");

                if (ImGui::RadioButton("Glow", aimbot_data.bUseGlow))
                {
                    aimbot_data.bUseGlow = true;
                    aimbot_data.bDrawParticles = false;
                }

                ImGui::SameLine(); DrawTools::DrawHelp("Dispatch Glow");

                {
                    static float color[]{   static_cast<float>(aimbot_data.cAimGlow[0]) / 255,
                                            static_cast<float>(aimbot_data.cAimGlow[1]) / 255,
                                            static_cast<float>(aimbot_data.cAimGlow[2]) / 255,
                                            static_cast<float>(aimbot_data.cAimGlow[3]) / 255 };
                    if (ImGui::ColorEdit4("Glow Color", color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8))
                        for (char8_t i = 0; i < SizeOfArray(color); i++)
                            aimbot_data.cAimGlow[i] = static_cast<char8_t>(color[i] * 255);
                    ImGui::SameLine(); DrawTools::DrawHelp("Custom Color");
                    ImGui::Dummy({ 5, 5 });
                }

                if (ImGui::RadioButton("Particles", aimbot_data.bDrawParticles))
                {
                    aimbot_data.bUseGlow = false;
                    aimbot_data.bDrawParticles = true;
                    if (aimbot_data.pGlowEnt)
                        aimbot_data.pGlowEnt->SetEntity(nullptr);
                }
                ImGui::SameLine(); DrawTools::DrawHelp("Particles to dispatch");

                static char particle[64]{ ')' };
                if (*particle == ')')
                    strncpy(particle, (*aimbot_data.sAimParticle).c_str(), SizeOfArray(particle));

                if (ImGui::InputText("##PARTICLE NAME", particle, 64))
                    *aimbot_data.sAimParticle = particle;

                ImGui::SameLine(); DrawTools::DrawHelp("Input particle name");
                ImGui::Dummy({ 5, 5 });
            }
            ImGui::EndChild();

            ImGui::DragFloat("Smooth", aimbot_data.flSmoothFactor.get(), 1.0, 0.0, 75.0, "%.0f");
            ImGui::SameLine(); DrawTools::DrawHelp("Aim Smooth rate [0.0, 75.0]");
        }

        ImGui::EndTabItem();
    }
}


void AimbotMenu::JsonCallback(Json::Value& json, bool read)
{
    Json::Value& aimbot = json["Aimbot"];
    Json::Value& effect = aimbot["Effects"];
    constexpr const char* _colors[]{ "Red", "Green", "Blue", "Alpha" };

    if (read)
    {
        PROCESS_JSON_READ(aimbot, "Enabled", Bool, aimbot_data.bIsEnabled);
        PROCESS_JSON_READ(aimbot, "Zoom-only", Bool, aimbot_data.bZoomedOnly);
        PROCESS_JSON_READ(aimbot, "AimLock", Bool, aimbot_data.bAimLock);
        PROCESS_JSON_READ(aimbot, "KeyLock", Bool, aimbot_data.bKeyLock);
        PROCESS_JSON_READ(aimbot, "AimKey", Int, aimbot_data.iAimKey);
        PROCESS_JSON_READ(aimbot, "Miss-Chance", Int, aimbot_data.iRandomMiss);
        PROCESS_JSON_READ(aimbot, "FOV", Float, aimbot_data.flAimFOV);
        PROCESS_JSON_READ(aimbot, "Smooth", Float, aimbot_data.flSmoothFactor);
        int tmp{};
        PROCESS_JSON_READ(aimbot, "TargetPriority", Int, tmp);
        aimbot_data.ePriority = static_cast<TargetPriority_t>(tmp);
        PROCESS_JSON_READ(aimbot, "HitboxPriority", Int, tmp);
        aimbot_data.eHitboxPos = static_cast<HitBoxPriority_t>(tmp);

        PROCESS_JSON_READ(effect, "Type", Int, tmp);
        switch (tmp)
        {
        case 0: 
        { 
            aimbot_data.bUseGlow = false;
            aimbot_data.bDrawParticles = false;
            if (aimbot_data.pGlowEnt)
                aimbot_data.pGlowEnt->SetEntity(nullptr);
            break;
        }
        case 1:
        {
            aimbot_data.bUseGlow = true;
            aimbot_data.bDrawParticles = false;
            break;
        }
        case 2:
        {
            aimbot_data.bUseGlow = false;
            aimbot_data.bDrawParticles = true;
            if (aimbot_data.pGlowEnt)
                aimbot_data.pGlowEnt->SetEntity(nullptr);
            break;
        }
        }

        PROCESS_JSON_READ(effect, "Particle Name", String, *aimbot_data.sAimParticle);

        int _clr[4];
        PROCESS_JSON_READ_COLOR(effect, "Glow Color", Int, _clr);
        for (char8_t i = 0; i < SizeOfArray(_clr); i++)
            aimbot_data.cAimGlow[i] = static_cast<char8_t>(_clr[i]);
    }
    else {
        PROCESS_JSON_WRITE(aimbot, "Enabled", *aimbot_data.bIsEnabled);
        PROCESS_JSON_WRITE(aimbot, "Zoom-only", *aimbot_data.bZoomedOnly);
        PROCESS_JSON_WRITE(aimbot, "AimLock", *aimbot_data.bAimLock);
        PROCESS_JSON_WRITE(aimbot, "KeyLock", *aimbot_data.bKeyLock);
        PROCESS_JSON_WRITE(aimbot, "AimKey", *aimbot_data.iAimKey);
        PROCESS_JSON_WRITE(aimbot, "Miss-Chance", *aimbot_data.iRandomMiss);
        PROCESS_JSON_WRITE(aimbot, "FOV", *aimbot_data.flAimFOV);
        PROCESS_JSON_WRITE(aimbot, "Smooth", *aimbot_data.flSmoothFactor);
        PROCESS_JSON_WRITE(aimbot, "TargetPriority", *aimbot_data.ePriority);
        PROCESS_JSON_WRITE(aimbot, "HitboxPriority", *aimbot_data.eHitboxPos);

        int tmp;
        if (!aimbot_data.bUseGlow)
            if (aimbot_data.bDrawParticles)
                tmp = 2;
            else tmp = 0;
        else tmp = 1;

        PROCESS_JSON_WRITE(effect, "Type", tmp);
        PROCESS_JSON_WRITE(effect, "Particle Name", *aimbot_data.sAimParticle);

        PROCESS_JSON_WRITE_COLOR(effect, "Glow Color", aimbot_data.cAimGlow.data());
    }
}

static AimbotMenu ___Aimbot;

#include "../Interfaces/HatCommand.h"
HAT_COMMAND(invalidate_aimlock, "Invalidates Aim-Lock")
{
    _InvalidateAimLock();
}