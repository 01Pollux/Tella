
#include "BasePlayer.hpp"
#include "winmain.hpp"
#include "GlobalHook/listener.hpp"

GAMEPROP_IMPL_RECV(ITFPlayerInternal, PlayerShared);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, PlayerClass);

GAMEPROP_IMPL_RECV(ITFPlayerInternal, DisguiseTeam);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, DisguiseClass);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, DisguiseTargetIndex);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, DisguiseHealth);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, DisguiseWeaponb);

GAMEPROP_IMPL_RECV(ITFPlayerInternal, MaxSpeed);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, CloakMeter);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, ChargeMeter);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, RageMeter);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, HypeMeter);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, EnergyDrinkMeter);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, RageDraining);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, FeignDeathReady);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, IsShieldEquipped);

GAMEPROP_IMPL_RECV(ITFPlayerInternal, EyeAngles);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, PunchAngle);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, ViewOffset);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, Velocity);

GAMEPROP_IMPL_RECV(ITFPlayerInternal, MyWeapons);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, ActiveWeapon);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, ObserverTarget);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, ObserverMode);

GAMEPROP_IMPL_RECV(ITFPlayerInternal, LifeState);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, Class);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, Streaks);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, CurrentHealth);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, DeathTime);

GAMEPROP_IMPL_RECV(ITFPlayerInternal, PlayerDominatingMe);
GAMEPROP_IMPL_RECV(ITFPlayerInternal, PlayerDominated);

class _TFCondProxy
{
public:
	void Change(ITFPlayerInternal* player, ETFCond tfcond, bool add)
	{
		if (!CondOffsets[0])
			Init(player);

		int bit{ };
		int type = static_cast<int>(tfcond) / 32;

		switch (type)
		{
		case 0:
		{
			bit = 1 << tfcond;
			uint32_t& cond = GetCond(player, 0);
			uint32_t& cond2 = GetCond(player, 1);

			if (add)
			{
				cond |= bit;
				cond2 |= bit;
			}
			else
			{
				cond &= ~bit;
				cond2 &= ~bit;
			}

			return;
		}

		case 1:
		{
			bit = 1 << (tfcond - 32);
			break;
		}

		case 2:
		{
			bit = 1 << (tfcond - 64);
			break;
		}

		case 3:
		{
			bit = 1 << (tfcond - 96);
			break;
		}

		case 4:
		{
			bit = 1 << (tfcond - 128);
			break;
		}
		}

		uint32_t& cond = GetCond(player, type + 1);

		if (add) cond |= bit;
		else cond &= ~bit;
	}

	bool InCond(const ITFPlayerInternal* player, ETFCond cond) noexcept
	{
		if (!CondOffsets[0])
			Init(player);

		int bit{ };
		int type = static_cast<int>(cond) / 32;

		switch (type)
		{
		case 0:
		{
			bit = 1 << cond;
			if (GetCond(player, 0) & bit || GetCond(player, 1) & bit)
				return true;
			else return false;
		}

		case 1:
		{
			bit = 1 << (cond - 32);
			break;
		}

		case 2:
		{
			bit = 1 << (cond - 64);
			break;
		}

		case 3:
		{
			bit = 1 << (cond - 96);
			break;
		}

		case 4:
		{
			bit = 1 << (cond - 128);
			break;
		}
		}

		return (GetCond(player, type + 1) & bit) != 0;
	}

private:
	uint32_t& GetCond(ITFPlayerInternal* player, int offset) const noexcept
	{
		return *(reinterpret_cast<uint32_t*>(reinterpret_cast<const uint32_t>(player) + CondOffsets[offset]));
	}
	
	const uint32_t GetCond(const ITFPlayerInternal* player, int offset) const noexcept
	{
		return *(reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint32_t>(player) + CondOffsets[offset]));
	}

	void Init(const ITFPlayerInternal* player)
	{
		constexpr const char* conds[]
		{
			"_condition_bits",
			"m_nPlayerCond",
			"m_nPlayerCondEx",
			"m_nPlayerCondEx2",
			"m_nPlayerCondEx3",
			"m_nPlayerCondEx4"
		};
		static_assert(std::extent_v<decltype(CondOffsets)> == SizeOfArray(conds));

		IGamePropHelper prop_finder;
		CachedRecvInfo info;
		const ClientClass* cls = player->GetClientClass();

		for (size_t i = 0; i < SizeOfArray(conds); i++)
		{
			prop_finder.FindRecvProp(cls, conds[i], &info);
			CondOffsets[i] = info.Offset;
		}
	}

	int CondOffsets[6]{ };
};
static _TFCondProxy TFCondProxy;


bool ITFPlayerInternal::InCond(ETFCond cond) const
{
	return TFCondProxy.InCond(this, cond);
}

void ITFPlayerInternal::AddCond(ETFCond cond, float duration)
{
	TFCondProxy.Change(this, cond, true);
}

void ITFPlayerInternal::RemoveCond(ETFCond cond)
{
	TFCondProxy.Change(this, cond, false);
}