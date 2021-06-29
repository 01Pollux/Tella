#include "Backtrack.hpp"

static BackTrack::MainHack backtrack;


MHookRes BackTrack::MainHack::PreCreateMove(const UserCmd* cmd)
{
	if (!enabled())
	{
		clear();
		return { };
	}

	ILocalPlayer pMe;
	if (pMe->LifeState != PlayerLifeState::Alive)
	{
		clear();
		return { };
	}

	mark_used();
	
	Latency_Stack = std::min(1.f, Latency_Stack + Interfaces::GlobalVars->IntervalPerTick);
	int tick = Interfaces::GlobalVars->TickCount;

	for (int i = 1; i <= Interfaces::GlobalVars->MaxClients; i++)
	{
		auto& info = RestoreData[i - 1];
		auto& data = (*info);

		ITFPlayer pPlayer(i);
		if (!pPlayer || pMe == pPlayer)
		{
			data.clear();
			continue;
		}

		if (pPlayer->LifeState != PlayerLifeState::Alive)
		{
			data.clear();
			continue;
		}

		if (data.size() >= RestoreData_t::SaveSize)
			data.pop_back();

		data.emplace_back(
			pPlayer->VecOrigin.get(),
			pPlayer->AngRotation.get(),

			pPlayer->SimulationTime.get(),
			pPlayer->AnimationTime.get(),
			pPlayer->Cycle.get(),
			pPlayer->Sequence.get(),

			tick
		);

		info.player() = pPlayer;
	}

	return { };
}


MHookRes BackTrack::MainHack::PostCreateMove(UserCmd* cmd)
{
	if (!enabled())
		return { };

	ILocalPlayer pMe;
	if (pMe->LifeState != PlayerLifeState::Alive)
		return { };

	const float latency = static_cast<float>(this->latency());

	auto is_valid_tick = [latency](int tick)
	{
		int target_tick = Interfaces::GlobalVars->TickCount + TimeToTicks(latency);
		int delta_tick = tick - target_tick - 2;
		return TicksToTime(delta_tick) <= 0.2f;
	};

	Vector my_pos{ pMe->VecOrigin };
	float nearest = FLT_MAX;

	using iterator = std::reverse_iterator<std::vector<RestoreData_t::Info>::const_iterator>;
	iterator backtrack_res;
	ITFPlayer pBest;

	for (const auto& ent_data : RestoreData)
	{
		const ITFPlayer pPlayer = ent_data.player();
		const auto& data = *ent_data;;

		for (auto iter = data.crbegin(); iter != data.crend(); iter++)
		{
			if (is_valid_tick(iter->Tick))
			{
				float cur = VectorHelper::GetFOV(my_pos, iter->Position, cmd->ViewAngles);
				if (cur < nearest)
				{
					nearest = cur;
					backtrack_res = iter;
					pBest = pPlayer;
				}
			}
		}
	}


	if (!pBest)
		return { };

	cmd->TickCount = backtrack_res->Tick;

	pBest->Sequence = backtrack_res->Sequence;
	pBest->Cycle = backtrack_res->Cycle;
	pBest->AnimationTime = backtrack_res->AnimationTime;
	pBest->SimulationTime = backtrack_res->SimulationTime;
	
	pBest->VecOrigin = backtrack_res->Position;
	pBest->AngRotation = backtrack_res->Angles;

	pBest->EFlags |= bitmask::to_mask(EntFlags::Dirty_Spatial_Partition, EntFlags::Dirty_Surroding_Collision_Bounds, EntFlags::Dirty_Abstransform);
	return { };
}


double BackTrack::MainHack::latency() const
{
	double latency = 0.;

	if (!Latency)
	{
		if (INetChannel* pNet = Interfaces::EngineClient->GetNetChannelInfo())
			latency = pNet->GetLatency(NetMsgFlowType::Outgoing);
	}

	return std::clamp(static_cast<double>(Latency.get()), 0., 1. - latency) * Latency_Stack;
}

#include "ConVar.hpp"
void _GetFlagsDummy()
{
	ILocalPlayer pMe;
	ReplyToCCmd(color::names::cyan, "{}", (void*)(pMe->EFlags.get()));
}
M01_CONCOMMAND(my_eflags, _GetFlagsDummy);