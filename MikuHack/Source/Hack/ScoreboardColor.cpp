
#include "../Interfaces/HatCommand.h"
#include "../Interfaces/NetMessage.h"
#include "../Main.h"
#include "../Helpers/VTable.h"

#include <array>

#include "../../Detour/detours.h"
#include "../../Helpers/DrawTools.h"

#include "../../Helpers/BytePatch.h"

class ScoreboardColor : public ExtraPanel
{
public:	//	ScoreboardColor
	AutoBool bEnabled	{ "ScoreboardColor::Enable", false };
	AutoBool bRandom	{ "ScoreboardColor::Random", false };
	AutoColor Colors[2]	{ { "ScoreboardColor::Color_RED"  }, 
						  { "ScoreboardColor::Color_BLUE" } };

public:	//	Globals::IGlobalHooks
	ScoreboardColor()
	{
		IGlobalEvent::LoadDLL::Hook::Register(std::bind(&ScoreboardColor::OnLoad, this));
		IGlobalEvent::UnloadDLL::Hook::Register(std::bind(&ScoreboardColor::OnUnload, this));

		Colors[0].Set({ DrawTools::ColorTools::Cyan<char8_t> });
		Colors[1].Set({ DrawTools::ColorTools::Fuschia<char8_t> });
	};

	HookRes OnLoad();
	HookRes OnUnload();

public:	//	ExtraPanel
	void OnRenderExtra() override final;
} static scoreboard_color;

std::vector<std::array<int, 3>> m_RainbowClr{ };

struct ColorData_t
{
	Color next_clr{ 255, 0, 255, 255 };

	void ScrambleColor(const std::array<int, 3>& RandomC, float ratio)
	{
		this->next_clr.SetColor(RandomC[0], RandomC[1], RandomC[2], 255 * ratio);
	}
	void ScrambleColor(float ratio, short team)
	{
		AutoColor& color = scoreboard_color.Colors[team];
		this->next_clr.SetColor(color[0], color[1], color[2], 255 * ratio);
	}

	[[noinline]] Color GetCurrent(bool bAlive, short team)
	{
		if (scoreboard_color.bRandom)
		{
			const auto& clr = m_RainbowClr[RandomInt(0, m_RainbowClr.size() - 1)];
			ScrambleColor(clr, bAlive ? 1.0:0.45);
		}
		else ScrambleColor(bAlive ? 1.0 : 0.45, team < 0 ? 0: team);

		return next_clr;
	}

} static m_ColorData[32];

static inline ColorData_t& GetClientColor(int index)
{
	return m_ColorData[index - 1];
}


DETOUR_CREATE_MEMBER(void, SetItemFgColor, int iItemIndex, Color cClr)
{
	{
		static uintptr_t* pWantedReturnAddress = reinterpret_cast<uintptr_t*>(Library::clientlib.FindPattern("UpdatePlayerList") + Offsets::ClientDLL::pUpdatePlayerList_ValidateRet);
		uintptr_t* pReturnAddress = reinterpret_cast<uintptr_t*>(_ReturnAddress());

		if (pReturnAddress != pWantedReturnAddress || !scoreboard_color.bEnabled)
		{
			DETOUR_MEMBER_CALL(SetItemFgColor, iItemIndex, cClr);
			return;
		}
	}
	KeyValues* pKVList;
	bool bAlive;
	_asm {
		push esi

		mov pKVList, esi

		mov esi, [ebp]
		mov bl, [esi-8h]
		mov bAlive, bl

		pop esi
	};
	
	int index = pKVList->GetInt("playerIndex");
	short team = GetClientEntityW(index)->GetTeam() - 2;

	ColorData_t& data = GetClientColor(index);
	Color c = data.GetCurrent(bAlive, team);

	DETOUR_MEMBER_CALL(SetItemFgColor, iItemIndex, c);
}

HookRes ScoreboardColor::OnLoad()
{
	std::array<int, 3> rand_clr;
	for (int i = 0; i < 139; i++)
	{
		for (int x = 0; x < 3; x++)
			rand_clr[x] = rand() % 255;

		m_RainbowClr.push_back(rand_clr);
	}

	uintptr_t ptr = Library::clientlib.FindPattern("SetItemFgColor");
	{ DETOUR_LINK_TO_MEMBER(SetItemFgColor, ptr); }

	return HookRes::Continue;
}

HookRes ScoreboardColor::OnUnload()
{
	DETOUR_UNLINK_FROM_MEMBER(SetItemFgColor); 
	return HookRes::Continue;
}

void ScoreboardColor::OnRenderExtra()
{
	if (ImGui::CollapsingHeader("Scoreboard Color"))
	{
		ImGui::Checkbox("Enable", bEnabled.get());
		ImGui::Checkbox("Fixed Colors", bRandom.get());
	}
}

HAT_COMMAND(scramble_data, "Scramble Players data")
{
	if (args.ArgC() != 2)
		REPLY_TO_TARGET(return, "%sscramble_data <size>\n", CMD_TAG);

	int size = atoi(args[1]);
	m_RainbowClr.clear();

	std::array<int, 3> rand_clr;
	for (int i = 0; i < size; i++)
	{
		for (int x = 0; x < 3; x++)
			rand_clr[x] = rand() % 255;

		m_RainbowClr.push_back(rand_clr);
	}
}