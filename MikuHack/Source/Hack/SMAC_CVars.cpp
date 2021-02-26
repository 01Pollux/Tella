
#include "../Main.h"
#include "../GlobalHook/load_routine.h"
#include "../GlobalHook/detour.h"

#include "../Interfaces/HatCommand.h"
#include "../Interfaces/NetMessage.h"
#include "../Interfaces/IVEngineClient.h"

#include "../Debug.h"
#include "../Helpers/String.h"
#include "../Helpers/Timer.h"

#include <set>

void ParseKeyValues(KeyValues*);

class SMAC_AntiCVars : public ExtraPanel, public IMainRoutine
{
public:	//	SMAC_AntiCVars
	std::unordered_map<std::string, std::string> m_CVarsList {
//		{ "sv_cheats",				"0.0" },
		{ "snd_visualize",			"0.0" },
		{ "snd_show",				"0.0" },
		{ "r_visocclusion",			"0.0" },
		{ "r_skybox",				"1.0" },
		{ "r_shadowwireframe",		"0.0" },
		{ "r_drawtranslucentworld",	"1.0" },
		{ "r_drawskybox",			"1.0" },
		{ "r_drawrenderboxes",		"0.0" },
		{ "r_drawparticles",		"1.0" },
		{ "r_drawothermodels",		"1.0" },
		{ "r_drawopaqueworld",		"1.0" },
		{ "r_drawentities",			"1.0" },
		{ "r_drawbeams",			"1.0" },
		{ "fog_enable",				"1.0" },
		{ "cl_interp",				"0.1" },
	};
	AutoBool bEnabled{ "SMAC_Cvars::Enable", false };

public:
	void OnLoadDLL() final
	{
		using namespace IGlobalDHookPolicy;
		auto send_net_msg = SendNetMsg::Hook::QueryHook(SendNetMsg::Name);
		send_net_msg->AddPreHook(HookCall::Any, std::bind(&SMAC_AntiCVars::OnSendNetMsg, this, std::placeholders::_1));
	};
	HookRes OnSendNetMsg(INetMessage&);

public:	//	ExtraPanel
	void OnRenderExtra() final
	{
		// Add input
		ImGui::Checkbox("Fake Query CVars", &bEnabled);
	}
} static smac_anti_cvars;


HookRes SMAC_AntiCVars::OnSendNetMsg(INetMessage& msg)
{
	if (!bEnabled)
		return HookRes::Continue;

	switch (msg.GetType())
	{
	case clc_RespondCvarValue:
	{
		CLC_RespondCvarValue& cvar_respond = reinterpret_cast<CLC_RespondCvarValue&>(msg);
		auto i = m_CVarsList.find(cvar_respond.m_szCvarName);

		if (i != m_CVarsList.end())
			cvar_respond.m_szCvarValue = i->second.c_str();
		break;
	}
	case clc_CmdKeyValues:
	{
		if (MIKUDebug::m_bDebugging)
			ParseKeyValues(static_cast<CLC_CmdKeyValues&>(msg).m_pKeyValues);
		break;
	}
	
	default: break;
	}

	return HookRes::Continue;
}

HAT_COMMAND(fake_cvar, "Set ConVar Value")
{
	if (args.ArgC() != 3)
		REPLY_TO_TARGET(return, "Usage: %sfake_cvar <name> <value>", CMD_TAG);

	const char* name = args[1];
	const char* value = args[2];

	ConVar* pCVar = g_pCVar->FindVar(name);
	if (!pCVar)
		REPLY_TO_TARGET(return, "Invalid ConVar: %s", name);

	pCVar->SetValue(value);
}

static void CollectUniqueKVSection(KeyValues* kv, std::set<KeyValues*>& output)
{
	FOR_EACH_SUBKEY(kv, subkey)
		FOR_EACH_VALUE(subkey, block)
			output.insert(block);

	FOR_EACH_VALUE(kv, block)
		output.insert(block);
}

void ParseKeyValues(KeyValues* kv)
{
	std::set<KeyValues*> kvs;
	CollectUniqueKVSection(kv, kvs);

	MIKUDebug::LogDebug(fmt::format("\n\nBEGIN KV: \"{}\"", kv->GetName()));

	using KVType = KeyValues::types_t;

	char dt = KVType::TYPE_NONE;
	const char* name = "";

	IBufferFormatter buf;
	
	buf.reserve(kvs.size() * 8);
	for (const KeyValues* kv : kvs)
	{
		dt = kv->m_iDataType;
		name = kv->GetName();
		buf.cat("\tKey: \"{}\" - ", name);

		switch (dt)
		{
		
		case KVType::TYPE_STRING:
		{
			if (kv->m_sValue && *(kv->m_sValue))
				buf.cat("STRING: {}", kv->m_sValue);
			else
				buf.cat("STRING: <EMPTY>");
			break;
		}

		case KVType::TYPE_INT:
		{
			buf.cat("INT: {}", kv->m_iValue);
			break;
		}

		case KVType::TYPE_UINT64:
		{
			buf.cat("DOUBLE: {}", *reinterpret_cast<double*>(kv->m_sValue));
			break;
		}

		case KVType::TYPE_FLOAT:
		{
			buf.cat("FLOAT: {}", kv->m_flValue);
			break;
		}
		case KVType::TYPE_COLOR:
		{
			buf.cat("COLOR: < {} - {} - {} - {} >", kv->m_Color[0], kv->m_Color[1], kv->m_Color[2], kv->m_Color[3]);
			break;
		}
		case KVType::TYPE_PTR:
		{
			buf.cat("PTR: ", kv->m_pValue);
			break;
		}
		default:
		{
			buf.cat("UNKNOWN");
			break;
		}
		}

		buf.cat("\n");
	}

	buf.cat("\nEND KV");
	MIKUDebug::LogDebug(std::move(*buf));
}

HAT_COMMAND(revive_marker, "")
{
	KeyValues* kv = new KeyValues("MVM_Revive_Response");
	kv->SetInt("accepted", 1);
	engineclient->ServerCmdKeyValues(kv);
}