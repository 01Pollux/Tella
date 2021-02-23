#include "../NetMessage.h"

#include "../IVEClientTrace.h"
IEngineTrace* clienttrace;
IVDebugOverlay* debugoverlay;


#include "../HatCommand.h"

HAT_COMMAND(string_cmd, "")
{
	INetChannel* pNet = static_cast<INetChannel*>(engineclient->GetNetChannelInfo());
	if (!pNet)
		REPLY_TO_TARGET(return, "Invalid NetChannel\n");

	const char* cmd = args.ArgS();

	NET_StringCmd str_cmd(cmd);
	pNet->SendNetMsg(str_cmd);
}

HAT_COMMAND(drop_netchan, "Disconnect with a custom reason")
{
	INetChannel* pNet = static_cast<INetChannel*>(engineclient->GetNetChannelInfo());
	if (pNet)
	{
		const char* cmd = args.ArgS();
		pNet->Shutdown(cmd);
	}
}