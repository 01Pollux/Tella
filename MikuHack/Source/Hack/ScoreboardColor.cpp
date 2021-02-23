
#include "../Main.h"
#include "../Interfaces/HatCommand.h"
#include "../GlobalHook/load_routine.h"

#include "../../Detour/detours.h"
#include "../../Helpers/DrawTools.h"
#include "../../Helpers/Commons.h"

#include <KeyValues.h>


class ScoreboardColor: public ExtraPanel, public IMainRoutine
{
public:	//	ScoreboardColor
	AutoBool bEnabled	{ "ScoreboardColor::Enable", false };
	AutoBool bRandom	{ "ScoreboardColor::Random", false };
	AutoColor Colors[2]	{ { "ScoreboardColor::Color_RED", DrawTools::ColorTools::Cyan<char8_t>  },
						  { "ScoreboardColor::Color_BLUE",DrawTools::ColorTools::Fuschia<char8_t> } };
public:	//	ExtraPanel
	void OnRenderExtra() final;

public:	//	IMainRoutine
	void OnLoadDLL() final;
	void OnUnloadDLL() final;

} static scoreboard_color;


struct _ColorData
{
	Color next_clr{ 255, 0, 255, 255 };
	static inline std::vector<std::array<int, 3>> RandomColors{ };

	void ScrambleColor(const std::array<int, 3>& color, float ratio)
	{
		this->next_clr.SetColor(color[0], color[1], color[2], 255 * ratio);
	}
	void ScrambleColor(float ratio, short team)
	{
		const auto& color = *scoreboard_color.Colors[team];
		this->next_clr.SetColor(color[0], color[1], color[2], 255 * ratio);
	}

	const Color& GetCurrent(bool is_player_alive, short team)
	{
		if (scoreboard_color.bRandom)
		{
			const auto& clr = RandomColors[Random::Int(0, RandomColors.size() - 1)];
			ScrambleColor(clr, is_player_alive ? 1.0 : 0.45);
		}
		else ScrambleColor(is_player_alive ? 1.0 : 0.45, team < 0 ? 0: team);

		return next_clr;
	}
} static ColorDdata[MAX_PLAYERS];

static inline _ColorData& GetClientColor(int index) noexcept
{
	return ColorDdata[index - 1];
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

	KeyValues* KVList;
	bool bAlive;
	_asm 
	{
		push ESI

		mov KVList, ESI

		mov ESI, [EBP]
		mov BL, [ESI-8h]
		mov bAlive, BL

		pop ESI
	};
	
	Color& c = cClr;
	int index = KVList->GetInt("playerIndex");
	if (ITFPlayer* pPlayer = ::GetITFPlayer(index))
	{
		int team = pPlayer->GetTeam() - 2;
		_ColorData& data = GetClientColor(index);
		c = data.GetCurrent(bAlive, team);
	}

	DETOUR_MEMBER_CALL(SetItemFgColor, iItemIndex, c);
}

void ScoreboardColor::OnLoadDLL()
{
	constexpr size_t NumColors = 39;
	std::array<int, 3> rand_clr;
	_ColorData::RandomColors.reserve(NumColors);

	for (int i = 0; i < NumColors; i++)
	{
		for (int x = 0; x < 3; x++)
			rand_clr[x] = Random::Int(0, 255);

		_ColorData::RandomColors.push_back(rand_clr);
	}

	uintptr_t ptr = Library::clientlib.FindPattern("SetItemFgColor");
	{ DETOUR_LINK_TO_MEMBER(SetItemFgColor, ptr); }
}

void ScoreboardColor::OnUnloadDLL()
{
	DETOUR_UNLINK_FROM_MEMBER(SetItemFgColor); 
}

void ScoreboardColor::OnRenderExtra()
{
	if (ImGui::CollapsingHeader("Scoreboard Color"))
	{
		ImGui::Checkbox("Enable", &bEnabled);
		ImGui::Checkbox("Random Colors", &bRandom);

		constexpr const char* teams[]{ "Red Team", "Blu Team" };
		for (char i = 0; i < 2; i++)
			ImGui::ColorEdit4_2(teams[i], *scoreboard_color.Colors[i]);
	}
}

HAT_COMMAND(scramble_data, "Scramble Players data")
{
	if (args.ArgC() != 2)
		REPLY_TO_TARGET(return, "%sscramble_data <size>\n", CMD_TAG);

	int size = atoi(args[1]);
	_ColorData::RandomColors.clear();

	std::array<int, 3> rand_clr;
	for (int i = 0; i < size; i++)
	{
		for (int x = 0; x < 3; x++)
			rand_clr[x] = Random::Int(0, 255);

		_ColorData::RandomColors.push_back(rand_clr);
	}
}