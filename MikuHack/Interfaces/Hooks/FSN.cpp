#include "../IBaseClientDLL.h"

#include "../Helpers/VTable.h"
#include "../Helpers/Offsets.h"

#include "../../Source/Debug.h"
#include "../NetMessage.h"

#include <iostream>

using bf_read_ref = bf_read&;

static void DBG_ProcessUserMsg(int, const bf_read&);

static AutoBool bDebugUserMessage("MIKUDebug.UserMsgDebug");

DECL_VHOOK_CLASS(void, FrameStageNotify, ClientFrameStage_t stage)
{
	IGlobalEvent::FrameStageNotify::Hook::RunAllHooks(stage);
	VHOOK_EXECUTE(FrameStageNotify, stage);
}

void _Dbg_OnDispatchMsg(const int& msg_type, const bf_read_ref& incoming)
{
	if (bDebugUserMessage)
		DBG_ProcessUserMsg(msg_type, incoming);
}

class FrameStageHook
{
	IUniqueVHook fsn_hook;
public:
	FrameStageHook() 
	{
		using namespace IGlobalEvent;
		LoadDLL::Hook::Register(
			[this]() -> HookRes
			{
				fsn_hook = std::make_unique<ISingleVHook>(clientdll, VHOOK_GET_CALLBACK(FrameStageNotify), IGlobalEvent::FrameStageNotify::Offset);
				VHOOK_LINK(fsn_hook, FrameStageNotify);
				return HookRes::Continue;
			});
		UnloadDLL::Hook::Register([this]() -> HookRes { fsn_hook = nullptr; return HookRes::Continue; });
	};
} static dummy_framehook;


#include <iomanip>
#include "../../Helpers/Timer.h"

void DBG_ProcessUserMsg(int msg_type, const bf_read& _maindata)
{
	bf_read data = _maindata;

	static player_info_t pinfo;
	static char pText[6][256];

	if(msg_type != UserMsg_t::HudMsg)
	{
		MIKUDebug::LogDebug(Format("\n\t<USER MSG>", msg_type));

		std::string msg;
		std::ostringstream str;
		while (data.GetNumBytesLeft())
			str << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data.ReadByte()) << ' ';

		msg = str.str();
		MIKUDebug::LogDebug(Format("\nDATA = [", msg, "]"));
		data.Seek(0);
		msg.clear();

		int pos = 0;
		while (data.GetNumBytesLeft())
		{
			if (char byte = data.ReadByte())
				msg.push_back(byte);
			else {
				MIKUDebug::LogDebug(Format("[", pos++, "] = \"", msg, "\""));
				msg.clear();
			}
		}

		data.Seek(0);
	}

	
	switch (msg_type)
	{
	case UserMsg_t::SayText:
	{
		int client = data.ReadByte();
		bool host_say = !engineclient->GetPlayerInfo(client, &pinfo);

		data.ReadString(pText[0], sizeof(pText[0]));
		bool bchat = data.ReadByte() ? true : false;

		MIKUDebug::LogDebug(Format("[UM: SayText] <", host_say ? "HOST" : pinfo.name, ">(", bchat, ") = \"", pText, "\""));
		break;
	}
	case UserMsg_t::SayText2:
	{
		int client = data.ReadByte();
		bool host_say = !engineclient->GetPlayerInfo(client, &pinfo);

		bool bchat = data.ReadByte();

		for(int i = 0; i < 6; i++)
			data.ReadString(pText[i], sizeof(pText[0]));

		MIKUDebug::LogDebug(Format(
							"\t[UM: SayText2] < ", host_say ? "HOST" : pinfo.name,">(", bchat,"): \"", pText[0],"\"\n"
							"\t\tName: ", pText[1], "\n"
							"\t\tText: ", pText[2], "\n"					
							"\t\tUnk3: ", pText[3], "\n"
							"\t\tUnk4: ", pText[4], "\n"
							"\t\tUnk5: ", pText[5]));

		break;
	}
	}
}

