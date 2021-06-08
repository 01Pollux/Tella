#include "GlobalHook/listener.hpp"
#include "cdll_int.hpp"
#include "SendNetMsg.hpp"
#include "KeyValues.hpp"

#include "Helper/Debug.hpp"
#include "Helper/Config.hpp"


DECL_DHOOK_HANDLER(SendNetMsg, M0PROFILER_GROUP::HOOK_SEND_NETMESSAGE, bool, INetMessage&, bool, bool);
EXPOSE_DHOOK(SendNetMsg, "SendNetMsg", M0Libraries::Engine->FindPattern("NetChan::SendNetMsg"));

class SendNetMsg_Mgr
{
public:
	SendNetMsg_Mgr()
	{
		M0EventManager::AddListener(
			EVENT_KEY_LOAD_DLL_EARLY,
			[this](M0EventData*)
			{
				SendNetMsg.init();
				SendNetMsg->AddPreHook(HookCall::Late, std::bind(&SendNetMsg_Mgr::OnSendNetMsg, this, std::placeholders::_1));
			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_RENDER_EXTRA,
			[this](M0EventData*)
			{
				if (ImGui::CollapsingHeader("Net Message debug"))
				{
					ImGui::Checkbox("Enabled", Enable.data());
					ImGui::SameLineHelp(Enable);
				}
				SendNetMsg.shutdown();
			},
			EVENT_NULL_NAME
		);

		M0EventManager::AddListener(
			EVENT_KEY_UNLOAD_DLL_LATE,
			[this](M0EventData*)
			{
				SendNetMsg.shutdown();
			},
			EVENT_NULL_NAME
		);
	}

private:
	HookRes OnSendNetMsg(const INetMessage&) const;

	void ParseKeyValues(const KeyValues* kv) const;

	SendNetMsg_Hook SendNetMsg;

	M0Config::Bool Enable{ "Debug.SendNetMsgs.Enable", false, "Debugging some incoming netmessages"};
} static sendnetmsg_hook;


HookRes SendNetMsg_Mgr::OnSendNetMsg(const INetMessage& msg) const
{
	if (Enable)
	{
		switch (msg.GetType())
		{
		case NetMsgType::CmdKeyValues:
		{
			ParseKeyValues(static_cast<const CLC_CmdKeyValues&>(msg).KV);
			break;
		}
		case NetMsgType::QueryCVarValue:
		{
			const CLC_RespondCvarValue& cvar = static_cast<const CLC_RespondCvarValue&>(msg);

			M0Logger::Dbg("Parsing cvar query from netmsg");

			M0Logger::Dbg("Name: {}", cvar.CvarName);
			M0Logger::Dbg("Value: {}", cvar.CvarValue);
			M0Logger::Dbg("Code: {}", static_cast<int>(cvar.StatusCode));
			M0Logger::Dbg("Cookie: {}\n", cvar.Cookie);

			break;
		}
		
		case NetMsgType::FileMD5Check:
		{
			const CLC_FileMD5Check& md5check = static_cast<const CLC_FileMD5Check&>(msg);

			M0Logger::Dbg("Parsing md5 check from netmsg");

			M0Logger::Dbg("PathID: {}", md5check.PathID);
			M0Logger::Dbg("Filename: {}", md5check.Filename);
			M0Logger::Dbg("MD5 Seed: {}\n", md5check.MD5.to_string());
			
			break;
		}
		
		case NetMsgType::FileCRCCheck:
		{
			const CLC_FileCRCCheck& crccheck = static_cast<const CLC_FileCRCCheck&>(msg);

			M0Logger::Dbg("Parsing crc check from netmsg");

			M0Logger::Dbg("PathID: {}", crccheck.PathID);
			M0Logger::Dbg("Filename: {}", crccheck.Filename);
			M0Logger::Dbg("MD5 Seed: {}", crccheck.MD5.to_string());
			M0Logger::Dbg("CRC Seed: {}", crccheck.CRCIOs);
			M0Logger::Dbg("Hash type: {}", crccheck.FileHashType);
			M0Logger::Dbg("File length: {}", crccheck.FileLen);
			M0Logger::Dbg("File number: {}", crccheck.PackFileNumber);
			M0Logger::Dbg("File id: {}", crccheck.PackFileID);
			M0Logger::Dbg("File freaction: {}\n", crccheck.FileFraction);
			
			break;
		}
		default: break;
		}
	}
	
	return HookRes::Continue;
}


void SendNetMsg_Mgr::ParseKeyValues(const KeyValues* kv) const
{
	class DmpCtx final : public IKeyValuesDumpContext
	{
	public:
		bool OnBeginKVSection(const KeyValues* pKey, int indentlvl) final
		{
			Tabs.resize(indentlvl);
			std::fill_n(Tabs.begin(), Tabs.size(), '\t');
			M0Logger::Dbg("{0}Begin of: \"{1}\"", Tabs, pKey->GetName());
			return true;
		}

		bool OnEndKVSection(const KeyValues* pKey, int indentlvl) final
		{
			Tabs.resize(indentlvl);
			std::fill_n(Tabs.begin(), Tabs.size(), '\t');
			M0Logger::Dbg("{0}Begin of: \"{1}\"", Tabs, pKey->GetName());
			return true;
		}

		bool OnKVSection(const KeyValues* pKey, int indentlvl) final
		{
			Tabs.resize(indentlvl);
			std::fill_n(Tabs.begin(), Tabs.size(), '\t');
			M0Logger::Dbg("{0}\"{1}\": \"{2}\"", Tabs, pKey->GetName(), pKey->GetString());
			return true;
		}

	private:
		std::string Tabs;
	} ctx;

	M0Logger::Dbg("Parsing KeyValues from netmsg");
	kv->Dump(&ctx);
}