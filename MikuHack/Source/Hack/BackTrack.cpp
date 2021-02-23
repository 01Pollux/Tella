#include "BackTrack.h"

#include <inetchannel.h>
#include "../Interfaces/IVEClientTrace.h"

#include "../Interfaces/IClientMode.h"

#define SIMULATION_TIME_PROP "m_flSimulationTime"

static IBackTrackHack track_hack;

struct {
	ITFPlayer* player;
	const IBackTrackData* data;

	constexpr operator bool() noexcept
	{
		return data != nullptr && player;
	}

	void set(ITFPlayer* player, const IBackTrackData* data) noexcept
	{
		this->player = player;
		this->data = data;
	}

	void reset() noexcept
	{
		player = nullptr;
		data = nullptr;
	}
} backtrack_data;


inline float GetLatency()
{
	INetChannel* nci = static_cast<INetChannel*>(engineclient->GetNetChannelInfo());
	return nci ? nci->GetLatency(FLOW_OUTGOING) + nci->GetLatency(FLOW_INCOMING) : 0.f;
}


HookRes IBackTrackHack::OnCreateMove(CUserCmd* cmd)
{
	if (!cmd || !cmd->command_number)
		return HookRes::BreakImmediate;

	if (!bEnable)
		return HookRes::Continue;

	ITFPlayer* pMe = ILocalPtr();
	if (pMe->GetLifeState() != LIFE_STATE::ALIVE)
		return HookRes::Continue;

	int tick = cmd->tick_count;
	int cur_tick = tick % SizeOfBacktrack;

	float best_fov = flTrackFOV;
	const float cur_time = gpGlobals->curtime;
	float correct_time = GetLerpTime() + GetLatency();
	correct_time = Clamp(correct_time, 0.0f, 1.0f);

	const IBackTrackData* best_data{ };
	ITFPlayer* best_ent{ };

	auto ProcessSingleData = [&best_data, &best_fov, &best_ent, 
							  tick, correct_time, cmd]
		(ITFPlayer* player, const IBackTrackData& data)
	{
		float delta_time = correct_time - (gpGlobals->curtime - TicksToTime(data.tick));

		if (fabsf(delta_time) > 0.2f)
			return;

		float cur = GetLocalFOV(cmd->viewangles, data.origin);
		if (cur < best_fov)
		{
			best_ent = player;
			best_fov = cur;
			best_data = &data;
		}
	};


	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		ITFPlayer* pPlayer = ::GetITFPlayer(i);
		IBackTrackInfo& data = backtrack[i - 1];

		if (BadEntity(pPlayer) || pPlayer->GetLifeState() != LIFE_STATE::ALIVE || pPlayer->GetTeam() == pMe->GetTeam())
		{
			data.reset();
			continue;
		}

		if (!data)
			data.init();

		IBackTrackData& info = data[cur_tick];

		info.tick = tick;
		info.simulation_time = *pPlayer->GetEntProp<float, PropType::Recv>(SIMULATION_TIME_PROP);
		info.origin = pPlayer->GetAbsOrigin();
		info.angles = pPlayer->GetAbsAngles();

		for (int i = 0; i <= SizeOfBacktrack; i++)
		{
			int pos = (cur_tick - i) % SizeOfBacktrack;
			const IBackTrackData& old_data = data[pos];

			if (old_data)
				ProcessSingleData(pPlayer, old_data);
		}
	}

	backtrack_data.set(best_ent, best_data);

	return HookRes::Continue;
}

/*
HookRes IBackTrackHack::OnAimbotResolvePlayer(QAngle& angle, const Vector* player_pos)
{
	backtrack_data.reset();

	if (!bEnable)
		return HookRes::Continue;

	ITFPlayer* pMe = ::ILocalPtr();
	ITFPlayer* pPlayer = Hook::GetReturnInfo();

	const IBackTrackData* best_data = nullptr;

	int index = pPlayer->entindex();

	IBackTrackInfo& data = backtrack[index - 1];

	if (!data)
		return HookRes::Continue;

	float bestFOV = flTrackFOV;
	const int cur_tick = current_cmd->tick_count;
	const float cur_time = gpGlobals->curtime;
	float correct_time = GetLerpTime() + GetLatency();
	correct_time = Clamp(correct_time, 0.0f, 1.0f);


	auto ProcessSingleData = [&best_data, &bestFOV, cur_tick,
							cur_time, correct_time, this]
	(const IBackTrackData& data)
	{
		float delta_time = correct_time - (cur_time - TicksToTime(data.tick));

		if (fabsf(delta_time) > 0.2f)
			return;

		float cur = GetLocalFOV(current_cmd->viewangles, data.origin);
		if (cur < bestFOV)
		{
			bestFOV = cur;
			best_data = &data;
		}
	};

	for (int i = 0; i < SizeOfBacktrack; i++)
	{
		// travel backward
		int pos = (cur_tick - i) % SizeOfBacktrack;

		const IBackTrackData& info = data[pos];
		if (info)
			ProcessSingleData(info);
	}

	should_reset = false;
	backtrack_data.set(pPlayer, best_data);

	if (best_data)
		angle = GetAimAngle(best_data->origin);
	
	return HookRes::Changed;
}
*/

void IBackTrackHack::OnLoadDLL()
{
	using namespace IGlobalVHookPolicy;

	auto createmove = CreateMove::Hook::QueryHook(CreateMove::Name);
	createmove->AddPreHook(HookCall::VeryEarly, std::bind(&IBackTrackHack::OnCreateMove, this, std::placeholders::_2));
	createmove->AddPostHook(HookCall::Late,
		[this](float, CUserCmd* cmd)
		{
			if (!cmd || !cmd->command_number)
				return HookRes::BreakImmediate;

			if (!bEnable)
				return HookRes::Continue;
			
			static Timer timer_update_lines;

			if (const auto& [player, data] = backtrack_data; backtrack_data)
			{
				ITFPlayer* pMe = ILocalPtr();

				int was = cmd->tick_count;

				cmd->tick_count = data->tick;
				*player->GetEntProp<float, PropType::Recv>(SIMULATION_TIME_PROP) = data->simulation_time;

				Vector& origin = *player->GetEntProp<Vector, PropType::Recv>("m_vecOrigin");
				Vector old{ origin };

				origin = data->origin;
//				*player->GetEntProp<QAngle>("m_vecAngles") = data->angles;

				if (timer_update_lines.trigger_if_elapsed(1800ms))
				{
					auto col = player->GetCollideable();
					const auto& mins = col->OBBMins();
					const auto& maxs = col->OBBMaxs();

					debugoverlay->AddBoxOverlay(origin, mins, maxs, { 0, 90.0f, 0 }, 0, 255, 0, 255, 2.0f);
					debugoverlay->AddBoxOverlay(old, mins, maxs, { 0, 90.0f, 0 }, 255, 0, 0, 255, 2.0f);

					std::cout << "Was: " << was << " Now: " << cmd->tick_count << "  Intern: " << data->tick << '\n';
				}
			}
			return HookRes::Continue;
		});
}

void IBackTrackHack::OnRenderExtra()
{
	if (ImGui::CollapsingHeader("BackTrack"))
	{
		ImGui::Checkbox("Enable", &bEnable);
		ImGui::DragFloat("FOV", &flTrackFOV, 1.f, 0.0f, 90.f);
	}
}

HAT_COMMAND(backtrack_toggle, "Toggles backtrack")
{
	AutoBool enable("BackTrack::bEnabled");
	enable = !enable;
	REPLY_TO_TARGET("BackTrack is now \"%s\"", enable ? "ON":"OFF");
}