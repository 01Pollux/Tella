#include "aimbot.hpp"
#include "aimbot_export.hpp"

#include "Helper/Random.hpp"

#include "DebugOverlay.hpp"
#include "Hooks/LevelChange.hpp"


static IAimbotHack dummy_aimbot;

IAimbotHack::IAimbotHack()
{
	M0EventManager::AddListener(EVENT_KEY_LOAD_DLL, std::bind(&IAimbotHack::OnLoadDLL, this), EVENT_NULL_NAME);

	M0EventManager::AddListener(EVENT_KEY_UNLOAD_DLL, std::bind(&IAimbotHack::OnUnloadDLL, this), EVENT_NULL_NAME);

	M0EventManager::AddListener(
		EVENT_KEY_ENTITY_DELETED, 
		[this](M0EventData* EventInfo) 
		{
			Vars.ParticleInfo.Handler_.delete_one(IBaseEntity(reinterpret_cast<IBaseEntityInternal*>(EventInfo)));
			//auto& data = IAimbotFilter::GetTargetNoReset();
			//if (data.target == pEnt)
			//	IAimbotFilter::Invalidate();
		}, 
		"Aimbot::OnEntityDeleted"
	);

	M0EventManager::AddListener(EVENT_KEY_RENDER_MENU, std::bind(&IAimbotHack::OnRender, this), "Aimbot::RenderMenu");
}

void IAimbotHack::OnLoadDLL()
{
	if (Interfaces::EngineClient->IsInGame())
		Vars.GlowInfo.Handler_.allocate();

	CreateMoveHandler.init();
	CreateMoveHandler->AddPreHook(HookCall::Any, std::bind(&IAimbotHack::OnCreateMovePre, this, std::placeholders::_2));
	CreateMoveHandler->AddPostHook(HookCall::Any, std::bind(&IAimbotHack::OnCreateMovePost, this, std::placeholders::_2));

	M0HookManager::Policy::LevelInit levelinit(true);
	levelinit->AddPostHook(HookCall::Any,
		[this](const char*)
		{
			Vars.GlowInfo.Handler_.allocate();
			return HookRes::Continue;
		}
	);

	M0HookManager::Policy::LevelShutdown levelshutdown(true);
	levelshutdown->AddPostHook(HookCall::Any,
		[this]()
		{
			Vars.GlowInfo.Handler_.destroy();
			return HookRes::Continue;
		}
	);
}

void IAimbotHack::OnUnloadDLL()
{
	Vars.GlowInfo.Handler_.destroy();
	Vars.ParticleInfo.Handler_.delete_all();
}

void IAimbotHack::DrawDebug(IBaseEntity pEnt, const Vector& aim_pos)
{
	const auto aimdebug = &Vars.Debug;
	const ICollideable* pCol = pEnt->CollisionProp.data();

	if (aimdebug->DrawBox)
	{
		NDebugOverlay::Box(
			pEnt->GetAbsOrigin(),
			pCol->OBBMins(),
			pCol->OBBMaxs(),
			0,
			231,
			255,
			255,
			aimdebug->RepeatTimer.get_time()
		);
	}
	
	if (aimdebug->DrawLine)
	{
		constexpr auto color = color::names::teal;

		NDebugOverlay::Line(
			ILocalPlayer()->EyePosition(),
			aim_pos,
			color.r,
			color.g,
			color.b,
			false,
			aimdebug->RepeatTimer.get_time()
		);
	}

	if (aimdebug->DrawHitbox)
	{
		pEnt->DrawHitboxes(aimdebug->RepeatTimer.get_time());
	}
}


HookRes IAimbotHack::OnCreateMovePre(UserCmd* cmd)
{
	using HookRes::Continue;

	ShouldProcess = false;

	if (!Vars.Enable)
	{
		Invalidate();
		return Continue;
	}

	PROFILE_SECTION("Aimbot::CreateMove_Pre", M0PROFILER_GROUP::CHEAT_PROFILE, cheat);
	PROFILE_SECTION("Aimbot::CreateMove_Pre", M0PROFILER_GROUP::HOOK_CREATE_MOVE, create_move);

	ILocalPlayer pMe;
	
	if (pMe->LifeState != PlayerLifeState::Alive)
	{
		Invalidate();
		return Continue;
	}

	else if (!CheckInputs(cmd))
	{
		Invalidate();
		return Continue;
	}

	else if (Vars.MissChance && Random::Int(0, 100) < Vars.MissChance)
		return Continue;
	
	auto& data = GetBestTarget(cmd);
	if (!data.validate(this, cmd))
	{
		Invalidate();
		return Continue;
	}

	Vars.TargetLock.Invalid_ = false;
	ShouldProcess = true;

	return Continue;
}


HookRes IAimbotHack::OnCreateMovePost(UserCmd* cmd)
{
	if (!ShouldProcess)
		return HookRes::Continue;

	ShouldProcess = false;

	auto& data = GetTargetNoReset();

	PROFILE_SECTION("Aimbot::CreateMove_Post", M0PROFILER_GROUP::CHEAT_PROFILE, cheat);
	PROFILE_SECTION("Aimbot::CreateMove_Post", M0PROFILER_GROUP::HOOK_CREATE_MOVE, create_move);

	QAngle res = VectorHelper::GetAimAngle(ILocalPlayer()->EyePosition(), data.results);

	bool do_override = false;
	if (!Vars.Smooth.get() || !Vars.Smooth.Override_.has_elapsed() || Vars.HitboxPriority == HitboxPriority::Head)
	{
		CreateMoveHandler->SetReturnInfo(false);
		*Interfaces::pSendPacket = false;
		do_override = true;
	}
	else
	{
		VectorHelper::SmoothAngle(cmd->ViewAngles, res, 1 / Vars.Smooth.get());
	}

	if (auto pEvent = M0EventManager::Find(EVENT_KEY_AIMBOT_SETAIM))
	{
		M0EventData aimevent;
		aimevent.set_ptr("target", &data.target);
		aimevent.set_ptr("view_angles", &res);
		aimevent.set_bool("is_player", data.is_player);

		pEvent(&aimevent);
	}

	QAngle old_angle = cmd->ViewAngles;
	cmd->ViewAngles = res;

	if (Vars.AutoShoot)
	{
		float x = (old_angle - res).x;
		if (x < 0)
			x = -x;

		if (x < 0.02f)
			cmd->Buttons |= IN_ATTACK;
	}

	// Particles
	{
		static AutoCTimer<0.13f> timer_update_particles;

		if (auto& info = Vars.GlowInfo;
			info.Enable)
		{
			info.Handler_.update(data.target, info.Color);
		}
		else if (auto& info = Vars.ParticleInfo;
			info.Enable && timer_update_particles.trigger_if_elapsed())
		{
			info.Handler_.update(data.target, info.String);
		}
	}

	// Debug
	{
		if (Vars.Debug.should_draw())
			DrawDebug(data.target, data.results);
	}

	return do_override ? HookRes::ChangeReturnValue : HookRes::Continue;
}

#include "ConVar.hpp"

static inline void Cmd_InvalidateAimbot()
{
	dummy_aimbot.Invalidate();
}	
M01_CONCOMMAND(aimbot_invalidate, Cmd_InvalidateAimbot, "Reset Aimbot State");