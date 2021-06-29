#pragma once

#include "Cheats/Main.hpp"

#include "GlobalHook/detour.hpp"
#include "Hooks/CreateMove.hpp"
#include "Helper/Debug.hpp"

#include "NetChannel.hpp"
#include "BitBuffers.hpp"
#include "UserCmd.hpp"

namespace BackTrack
{
	struct RestoreData_t
	{
		static constexpr size_t SaveSize = 64U;
		struct Info
		{
			Vector	Position;
			QAngle	Angles;

			float	SimulationTime;
			float	AnimationTime;
			float	Cycle;
			int		Sequence;

			int		Tick;
		};

		const auto& operator*() const noexcept { return Data; }
		auto& operator*() noexcept { return Data; }

		const ITFPlayer& player() const noexcept { return pPlayer; }
		ITFPlayer& player() noexcept { return pPlayer; }

	private:
		ITFPlayer pPlayer;
		std::vector<Info> Data;
	};
	
	class MainHack
	{
	public:
		MainHack();

		void OnSendDatagram(INetChannel*) const;

		_NODISCARD bool enabled() const noexcept { return Enable && !Cleared; }
		void clear()
		{
			if (!Cleared)
			{
				for (auto& info : RestoreData)
					(*info).clear();

				Cleared = true;
			}
			Latency_Stack = 0.f;
		}

		void mark_used() noexcept { Cleared = false; }

	private:
		MHookRes PreCreateMove(const UserCmd*);
		MHookRes PostCreateMove(UserCmd*);

		void UpdateDatagram();

		_NODISCARD double latency() const;
		_NODISCARD RestoreData_t&
			data(size_t i) noexcept { return RestoreData[i - 1]; }


		M0Config::Bool Enable{ "Backtrack.Enable", false, "Enable player backtracking" };
		M0Config::Float Latency{ "Backtrack.Latency", 0.f, "Set Backtrack Latency" };


		std::array<RestoreData_t, MAX_PLAYERS - 1> RestoreData;
		bool Cleared{ true };
		float Latency_Stack{ 0.f };
	};
}
