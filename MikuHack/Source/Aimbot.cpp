
#include "Main.h"

#include "../Interfaces/IVEClientTrace.h"
#include "../Interfaces/IClientListener.h"
#include "../Interfaces/IClientMode.h"

#include "../Helpers/DrawTools.h"
#include "../GlobalHook/vhook.h"
#include "../GlobalHook/load_routine.h"

#include "Hack/Effects.h"
#include "../Interfaces/Glow.h"

#include "../Profiler/mprofiler.h"


class IAimbotHack : public MenuPanel,
                    public IMainRoutine,
                    public IClientEntityListener
{
public:	// MenuPanel
    void OnRender() final;
    void JsonCallback(Json::Value& json, bool read) final;

public: // IMainRoutine
    void OnLoadDLL() final;
    void OnUnloadDLL() final;

public: // IClientEntityListener
    void OnEntityDeleted(IClientShared* pEnt) final;

public: // IAimbotHack
    HookRes OnCreateMove(CUserCmd* cmd);

private:
    IGlobalVHook<bool, float, CUserCmd*>* CreateMove;
} static dummy_aimbot;



static IBaseHandle pHndl;
static ITFPlayer* pLastEnt = nullptr;
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
    AutoColor cAimGlow{ "Aimbot::GlowColor",        DrawTools::ColorTools::Cyan<char8_t> };

    AutoInt iAimKey{ "Aimbot::AimKey",          VK_DELETE };
    AutoInt iRandomMiss{ "Aimbot::RandomMiss",  21 };

    AutoInt ePriority{ "Aimbot::ePriority",     CLOSEST_FOV };
    AutoInt eHitboxPos{ "Aimbot::eHitboxPos",   HB_CUSTOM };

    AutoFloat flAimFOV{ "Aimbot::AimFOV",               45.f };
    AutoFloat flSmoothFactor{ "Aimbot::SmoothFactor",   29.f };

    AutoString sAimParticle{ "Aimbot::AimParticle", "soldierbuff_blue_buffed" };

    IUniqueGlowObject pGlowEnt = nullptr;

    bool bShouldAim = true;

    _AimDataInfo() noexcept
    {
        (*sAimParticle).reserve(SizeOfAimParticle());
    }

    inline constexpr void ToggleState(bool state) noexcept
    {
        this->bShouldAim = state;
    }

    inline constexpr bool CanAim() noexcept
    {
        return this->bShouldAim;
    }

    inline void SetKey(int i) noexcept
    {
        switch (i)
        {
        case 0:
        {
            iAimKey = static_cast<int>('Q');
            break;
        }
        case 1:
        {
            iAimKey = VK_END;
            break;
        }
        case 2:
        {
            iAimKey = VK_F9;
            break;
        }
//        case 3:
        default:
        {
            iAimKey = VK_DELETE;
            break;
        }
        }
    }

    inline int KeyToPos() noexcept
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
		default:
            return 3;
        }
    }

    inline void AllocateGlowOnce()
    {
        if (!pGlowEnt)
            pGlowEnt = std::make_unique<IGlowObject>();
    }

    inline void DestroyGlowOnce() noexcept
    {
        pGlowEnt = nullptr;
    }

    constexpr size_t SizeOfAimParticle() const
    {
        return 64;
    }
} aimbot_data;

static std::unordered_map<ITFPlayer*, TimerID> m_MapTimer;


static bool IsProjectileBased(ClassID clsID) noexcept
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

static bool IsSniperRifle(ClassID clsID) noexcept
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

inline void _InvalidateAimLock() noexcept
{
    m_bFoundTarget = false;
    pLastEnt = nullptr;
    aimbot_data.pGlowEnt->SetEntity(nullptr);
}


static bool FindBestTarget(const QAngle& angle);
static bool IsValidPlayer(ITFPlayer*);
static bool GetBestPlayerPos(ITFPlayer*, const QAngle&, Vector*);
static inline void SmoothAngle(const QAngle&, QAngle&, float);


struct KillParticleInfo
{
    CTFParticle* pParticle;
    CTFParticleFactory* pFactory;
    ITFPlayer* pOwner;
};

void IAimbotHack::OnEntityDeleted(IClientShared* pEnt)
{
    auto i = m_MapTimer.find(static_cast<ITFPlayer*>(pEnt));
    if (i != m_MapTimer.end())
        Timer::DeleteFuture(i->second, true);
}

HookRes IAimbotHack::OnCreateMove(CUserCmd* cmd)
{
    AutoBool("BackTrack::Reset") = true;

    if (!aimbot_data.bIsEnabled)
        return HookRes::Continue;

    M0Profiler watch_aimbot("IAimbotHack::CreateMove", M0PROFILER_GROUP::CHEAT_PROFILE);
    static bool good_local = false;
    static Timer timer_refresh_stat;
    if (timer_refresh_stat.trigger_if_elapsed(100ms))
        good_local = engineclient->IsInGame();

    ITFPlayer* pMe = ::ILocalPtr();
    if (!pMe || !good_local)
        return HookRes::BreakImmediate;
    else if (pMe->GetLifeState() != LIFE_STATE::ALIVE)
        return HookRes::Continue;

    class IAimState
    {
        bool invalid{ false };
    public:

        constexpr operator bool() noexcept
        {
            return invalid;
        }
        const void invalidate() noexcept
        {
            _InvalidateAimLock();
            invalid = true;
        }
        constexpr void set_state(bool state) noexcept
        {
            invalid = state;
        }
    } static _aim_state;

    if (!(cmd->buttons & IN_ATTACK) && !aimbot_data.bAutoAim)
    {
        if (!_aim_state)
            _aim_state.invalidate();
        return HookRes::Continue;
    }

    //Update ClassID && Key
    {
        static Timer timer_update_key;
        bool bKeyPress = GetAsyncKeyState(aimbot_data.iAimKey);

        if (aimbot_data.bKeyLock)         aimbot_data.ToggleState(bKeyPress);
        else if (bKeyPress && timer_update_key.has_elapsed(500ms))
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
    IBaseObject* pWep = ::GetIBaseObject(pMe->GetActiveWeapon());
    ClassID clsID = pWep ? static_cast<ClassID>(pWep->GetClientClass()->m_ClassID) : ClassID_CTFPlayer;
    static Timer timer_snap_track;
    {
        if (aimbot_data.bZoomedOnly && IsSniperRifle(clsID))
            if (pMe->InCond(ETFCond::TF_COND_ZOOMED))
                timer_snap_track.update();
    }

    if (aimbot_data.iRandomMiss && (Random::Int(0, 100) < aimbot_data.iRandomMiss))
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }

    if (!FindBestTarget(cmd->viewangles))
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }
    
    Vector vecBestPos;
    if (!GetBestPlayerPos(pLastEnt, cmd->viewangles, &vecBestPos))
    {
        _InvalidateAimLock();
        return HookRes::Continue;
    }

    _aim_state.set_state(false);

    QAngle angRes = GetAimAngle(vecBestPos);

    bool override = false;
    if (aimbot_data.flSmoothFactor && timer_snap_track.has_elapsed(300ms))
        SmoothAngle(cmd->viewangles, angRes, aimbot_data.flSmoothFactor);
    else {
        CreateMove->SetReturnInfo(false);
        override = true;
    }

    cmd->viewangles = angRes;

    {
        static Timer update_line;
        if (MIKUDebug::m_bDebugging && update_line.trigger_if_elapsed(1800ms))
            debugoverlay->AddLineOverlay(pMe->LocalEyePosition(), vecBestPos, 255, 255, 0, false, 2.0f);
    }

    if (aimbot_data.bUseGlow)
    {
        aimbot_data.pGlowEnt->SetEntity(pLastEnt);
        aimbot_data.pGlowEnt->SetColor(DrawTools::ColorTools::FromArray(*aimbot_data.cAimGlow));
    }
    else if (aimbot_data.bDrawParticles && pLastEnt)
    {
        // Display Particles
        static Timer timer_update_particles;
        if (timer_update_particles.trigger_if_elapsed(130ms))
        {
            auto pos = m_MapTimer.find(pLastEnt);

            if (pos != m_MapTimer.end())
                Timer::RewindBack(pos->second);
            else {
                auto factory = CTFParticleFactory::ParticleProp(pLastEnt);

                KillParticleInfo* pInfo = new KillParticleInfo{
                            factory->Create((*aimbot_data.sAimParticle).c_str(), PATTACH_ABSORIGIN_FOLLOW),
                            factory,
                            pLastEnt
                };

                TimerID particle_timer = Timer::CreateFuture(150ms, TimerFlags::ExecuteOnMapEnd, 
                    [](void* data)
                    {
                        KillParticleInfo* pInfo = reinterpret_cast<KillParticleInfo*>(data);
                        pInfo->pFactory->StopEmission(pInfo->pParticle);
                        m_MapTimer.erase(pInfo->pOwner);
                        delete pInfo;
                    }, pInfo);

                m_MapTimer.insert(std::make_pair(pLastEnt, particle_timer));
            }
        }
    }

    return override ? HookRes::ChangeReturnValue : HookRes::Continue;
}

void IAimbotHack::OnLoadDLL()
{
    using namespace IGlobalVHookPolicy;

    if (engineclient->IsInGame())
        aimbot_data.AllocateGlowOnce();

    if (!BadLocal())
    {
        aimbot_data.AllocateGlowOnce();
    }

    CreateMove = CreateMove::Hook::QueryHook(CreateMove::Name);
    CreateMove->AddPostHook(HookCall::VeryEarly, std::bind(&IAimbotHack::OnCreateMove, this, std::placeholders::_2));
            
    auto level_init = LevelInit::Hook::QueryHook(LevelInit::Name);
    level_init->AddPostHook(HookCall::Any, 
        [this](const char*)
        {
            AddEntityListener();
            aimbot_data.AllocateGlowOnce(); 
            return HookRes::Continue; 
        }
    );

    auto level_shutdown = LevelShutdown::Hook::QueryHook(LevelShutdown::Name);
    level_shutdown->AddPostHook(HookCall::Any, 
        [this]() 
        {
            RemoveEntityListener();
            aimbot_data.DestroyGlowOnce();
            return HookRes::Continue;
        }
    );
}

void IAimbotHack::OnUnloadDLL()
{
    RemoveEntityListener();
    aimbot_data.DestroyGlowOnce();
}

static bool IsValidPlayer(ITFPlayer* pPlayer)
{
    if (pPlayer->GetLifeState() != LIFE_STATE::ALIVE)
        return false;

    IBaseObject* pActiveWep = ::ILocalWpn();

    const Vector& vecPos = pPlayer->GetAbsOrigin();

    // if we have active melee
    {
        if (pActiveWep->GetWeaponSlot() == 2)
            if (vecPos.DistTo(::ILocalPtr()->GetAbsOrigin()) > pActiveWep->Melee_GetSwingRange())
                return false;
    }

    //filter conds
    {
        constexpr ETFCond BadConds[]{
            TF_COND_STEALTHED_USER_BUFF,
            TF_COND_BULLET_IMMUNE,
            TF_COND_INVULNERABLE,
            TF_COND_DEFENSEBUFF,
            TF_COND_DEFENSEBUFF_HIGH,
            TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
            TF_COND_FEIGN_DEATH,
            TF_COND_DISGUISED,
        };

        for (ETFCond cond : BadConds)
        {
            if (pPlayer->InCond(cond))
                return false;
        }
    }

    return true;
}


bool FindBestTarget(const QAngle& angle)
{
    if (aimbot_data.bAimLock && m_bFoundTarget)
        if (pHndl.IsValid() && IsValidPlayer(pLastEnt))
            return true;
        else m_bFoundTarget = false;

    float flCur = 0.0, flBest = 99999.9f;
    ITFPlayer* pBest = nullptr;
    ITFPlayer* pMe = ::ILocalPtr();
    const Vector& pEyeVec = pMe->LocalEyePosition();

    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        ITFPlayer* pCur = ::GetITFPlayer(i);

        if (::BadEntity(pCur))
            continue;

        if (pCur->GetTeam() == pMe->GetTeam())
            continue;

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

            case TargetPriority_t::LOWEST_HP:
                flCur = static_cast<float>(*pCur->GetEntProp<int, PropType::Recv>("m_iHealth"));
                break;

            case TargetPriority_t::HIGHEST_HP:
                flCur = -static_cast<float>(*pCur->GetEntProp<int, PropType::Recv>("m_iHealth"));
                break;

            case TargetPriority_t::HIGHEST_SCORE:
                flCur = -static_cast<float>(ctfresource.GetEntProp<int>(pCur, "m_iScore"));
                break;

//            case TargetPriority_t::CLOSEST_FOV:
            default:
                flCur = GetLocalFOV(angle, pCur->EyePosition());
                if (flCur > aimbot_data.flAimFOV)
                    continue;
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


static bool GetClosestHitbox(IClientShared* pEnt, const QAngle& angle, Vector*& vec, std::unique_ptr<IBoneCache> cache)
{
    const model_t* mdl = pEnt->GetModel();
    if (!mdl)
        return false;

    studiohdr_t* pStudioHDR = modelinfo->GetStudiomodel(mdl);
    if (!pStudioHDR)
        return false;

    float nearest = 99999999.f;

    mstudiohitboxset_t* pStudioBoxSet = pStudioHDR->pHitboxSet(pEnt->HitboxSet());

    Vector hitbox_pos, vecFinal = vec3_origin;
    ITFPlayer* pMe = ::ILocalPtr();
    Vector eye_pos = pMe->LocalEyePosition();

    for (int i = 0; i < pStudioBoxSet->numhitboxes; i++)
    {
        if (!pEnt->GetHitbox(i, cache.get()))
            continue;

        hitbox_pos = cache->center;
        if (!Trace::VectorIsVisible(eye_pos, hitbox_pos, pMe))
            continue;

        float cur = GetLocalFOV(angle, hitbox_pos);
        if (cur < nearest && cur <= aimbot_data.flAimFOV)
        {
            vecFinal = hitbox_pos;
            nearest = cur;
        }
    }

    *vec = vecFinal;
    return vecFinal != vec3_origin;
}

bool GetBestPlayerPos(ITFPlayer* pEnt, const QAngle& angle, Vector* vecFinal)
{
    ITFPlayer* pMe = ::ILocalPtr();
    IBaseObject* pActiveWeapon = ::GetIBaseObject(pMe->GetActiveWeapon());

    std::unique_ptr<IBoneCache> cache = std::make_unique<IBoneCache>();

    switch (aimbot_data.eHitboxPos)
    {
    case HitBoxPriority_t::HB_HEAD:
    {
        if (pEnt->GetHitbox(hitbox_t::head, cache.get()))
        {
            Vector vecRes = cache->center;

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
            float flDmg = *pActiveWeapon->GetEntProp<float, PropType::Recv>("m_flChargedDamage");
            if (flDmg >= *(pEnt->GetEntProp<int, PropType::Recv>("m_iHealth")))
                hitbox = hitbox_t::spine_3;
            else
                hitbox = hitbox_t::head;
            m_bCanSnap = true;

            if (hitbox != hitbox_t::invalid)
            {
                if (pEnt->GetHitbox(hitbox, cache.get()))
                {
                    const Vector& vecRes = cache->center;
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
                const Vector&  vecRes = cache->center;
                if (Trace::VectorIsVisible(pMe->LocalEyePosition(), vecRes, pMe))
                {
                    *vecFinal = vecRes;
                    return true;
                }
            }
            if (pEnt->GetHitbox(hitbox_t::hip_R, cache.get()))
            {
                const Vector& vecRes = cache->center;
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
        return GetClosestHitbox(pEnt, angle, vecFinal, std::move(cache));
    }
    default:
        break;
    }
}

void SmoothAngle(const QAngle& input, QAngle& output, float factor)
{
    QAngle delta = (input - output) / factor;

    output.x = ApproachAngle(output.x, input.x, delta.x);
    output.y = ApproachAngle(output.y, input.y, delta.y);

    ClampAngle(output);
}


void IAimbotHack::OnRender()
{
    if (ImGui::BeginTabItem("Aimbot"))
    {
        ImGui::Checkbox("Enable", &aimbot_data.bIsEnabled);

        ImGui::Checkbox("Auto Aim", &aimbot_data.bAutoAim);
        ImGui::SameLine(); ImGui::DrawHelp("Only activate if MOUSE1 is pressed");

        if (aimbot_data.bIsEnabled)
        {
            {
                ImGui::Combo("Priority Types", &aimbot_data.ePriority, "Highest Score\0Closest FOV\0Lowest HP\0Highest HP\0Furthest Away\0\0");
                ImGui::SameLine(); ImGui::DrawHelp("Choose your player priority");
                ImGui::Separator();
            }

            {
                ImGui::Combo("Aim Priority", &aimbot_data.eHitboxPos, "Head\0Closest\0Custom\0\0");
                ImGui::SameLine(); ImGui::DrawHelp("Choose your hitbox priority");
                ImGui::Separator();
            }

            {
                static int m_iCur; m_iCur = aimbot_data.KeyToPos();
                if (ImGui::Combo("Switch Key", &m_iCur, "Q\0END\0F9\0DELETE\0\0"))
                    aimbot_data.SetKey(m_iCur);

                ImGui::SameLine(); ImGui::DrawHelp("Toggle Aimbot State");
                ImGui::Separator();
            }

            ImGui::Checkbox("Zoomed", &aimbot_data.bZoomedOnly);
            ImGui::SameLine(); ImGui::DrawHelp("Sniper: Only activate when you are scoping");
            ImGui::Dummy({ 5, 5 });

            ImGui::Checkbox("Aim-Lock", &aimbot_data.bAimLock);
            ImGui::SameLine(); ImGui::DrawHelp("Keep targeting same player (until he is dead / aim reset)");
            ImGui::Dummy({ 5, 5 });

            ImGui::InputInt("Random Miss", &aimbot_data.iRandomMiss);
            ImGui::SameLine(); ImGui::DrawHelp("Chance of randomly missing target");
            ImGui::Dummy({ 5, 5 });

            ImGui::InputFloat("FOV", &aimbot_data.flAimFOV, 1.0f, 5.0f, "%.0f");
            ImGui::SameLine(); ImGui::DrawHelp("Field of view to target");
            ImGui::Dummy({ 5, 5 });

            ImGui::Checkbox("Key-Lock", &aimbot_data.bKeyLock);
            ImGui::SameLine(); ImGui::DrawHelp("Keep pressing Switch Key for aim to activate");
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
                ImGui::SameLine(); ImGui::DrawHelp("No Particle");

                if (ImGui::RadioButton("Glow", aimbot_data.bUseGlow))
                {
                    aimbot_data.bUseGlow = true;
                    aimbot_data.bDrawParticles = false;
                }

                ImGui::SameLine(); ImGui::DrawHelp("Dispatch Glow");

                {
                    ImGui::ColorEdit4_2("Glow Color", *aimbot_data.cAimGlow, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Uint8);
                    ImGui::SameLine(); ImGui::DrawHelp("Custom Color");
                    ImGui::Dummy({ 5, 5 });
                }

                if (ImGui::RadioButton("Particles", aimbot_data.bDrawParticles))
                {
                    aimbot_data.bUseGlow = false;
                    aimbot_data.bDrawParticles = true;
                    if (aimbot_data.pGlowEnt)
                        aimbot_data.pGlowEnt->SetEntity(nullptr);
                }

                ImGui::SameLine(); ImGui::DrawHelp("Particles to dispatch");

                ImGui::InputText("##PARTICLE NAME", (*aimbot_data.sAimParticle).data(), aimbot_data.SizeOfAimParticle());

                ImGui::SameLine(); ImGui::DrawHelp("Input particle name");
                ImGui::Dummy({ 5, 5 });
            }
            ImGui::EndChild();

            ImGui::DragFloat("Smooth", &aimbot_data.flSmoothFactor, 1.0, 0.0, 75.0, "%.0f");
            ImGui::SameLine(); ImGui::DrawHelp("Aim Smooth rate [0.0, 75.0]");
        }

        ImGui::EndTabItem();
    }
}


void IAimbotHack::JsonCallback(Json::Value& json, bool read)
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
//        case 2:
        default:
        {
            aimbot_data.bUseGlow = false;
            aimbot_data.bDrawParticles = true;
            if (aimbot_data.pGlowEnt)
                aimbot_data.pGlowEnt->SetEntity(nullptr);
            break;
        }
        }

        PROCESS_JSON_READ(effect, "Particle Name", String, *aimbot_data.sAimParticle);

        int _clr[4]{ };
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

        PROCESS_JSON_WRITE_COLOR(effect, "Glow Color", &aimbot_data.cAimGlow);
    }
}


#include "../Interfaces/HatCommand.h"
HAT_COMMAND(invalidate_aimlock, "Invalidates Aim-Lock")
{
    _InvalidateAimLock();
}

HAT_COMMAND(realloc_glow, "Reallocate Glow")
{
    if (!aimbot_data.pGlowEnt)
    {
        aimbot_data.AllocateGlowOnce();
        REPLY_TO_TARGET(NULL, "Allocating glow");
    }
}

HAT_COMMAND(eye_pos_test, "")
{
    auto pMe = ILocalPtr();
    ITFParticleData* data = new ITFParticleData(pMe, "ghost_smoke", ParticleAttachment_t::PATTACH_ABSORIGIN_FOLLOW, NULL, pMe->EyePosTest());

    Timer::CreateFuture(3s, TimerFlags::ExecuteOnMapEnd, [](void* info) {
        ITFParticleData* particle = (ITFParticleData*)info;
        delete particle;
    }, data);
}