#include "cdll_int.hpp"
#include "NetChannel.hpp"
#include "NetMessage.hpp"

#include "VGUI.hpp"
#include "ConVar.hpp"
#include "Helper/Format.hpp"


static void Cmd_LeaveMatch(const CCommand& args)
{
	INetChannel* pNet = Interfaces::EngineClient->GetNetChannelInfo();
	if (pNet)
		pNet->Shutdown(args.str());
}
M01_CONCOMMAND(disconnect, Cmd_LeaveMatch, "Disconnect with custom reason");


static void Cmd_LeaveMatchVAC()
{
	INetChannel* pNet = Interfaces::EngineClient->GetNetChannelInfo();
	if (pNet)
		pNet->Shutdown("VAC banned from secure server");
}
M01_CONCOMMAND(disconnect_vac, Cmd_LeaveMatchVAC, "Disconnect with vac reason");


static void Cmd_Say(const CCommand& args)
{
	std::string txt = std::format("say {}", args.str());
	FixEscapeCharacters(txt);
	Interfaces::EngineClient->ServerCmd(txt.c_str());
}
M01_CONCOMMAND(say, Cmd_Say, "Say to game text chat");


static void Cmd_ChangeName(const CCommand& args)
{
	INetChannel* pNet = Interfaces::EngineClient->GetNetChannelInfo();
	if (pNet)
	{
		NET_SetConVar name("name", args.str());
		name.Reliable = false;
		pNet->SendNetMsg(name);
	}
}
M01_CONCOMMAND(name, Cmd_ChangeName, "Change player's name");


static void Cmd_ReloadCursor()
{
	Interfaces::VGUISurface->LockCursor();
	Interfaces::VGUISurface->SetCursorAlwaysVisible(false);
}	
M01_CONCOMMAND(cursor_reload, Cmd_ReloadCursor, "Reload cursor state (visible in game)");
