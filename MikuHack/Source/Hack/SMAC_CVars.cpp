
#include "../Interfaces/HatCommand.h"
#include "../Interfaces/NetMessage.h"
#include "../Main.h"

#include "../Helpers/VTable.h"
#include "../Detour/detours.h"

void ParseKeyValues(KeyValues*);

class SMAC_AntiCVars : public ExtraPanel
{
public:	//	SMAC_AntiCVars
	std::unordered_map<std::string, std::string> m_CVarsList {
//		{ "sv_cheats",				"0.0" },
//		{ "sv_footsteps",			"1.0" },	
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

public:	//	Globals::IGlobalHooks
	SMAC_AntiCVars() 
	{
		IGlobalEvent::SendNetMsg::Hook::Register(std::bind(&SMAC_AntiCVars::OnSendNetMsg, this, std::placeholders::_1));
	};
	HookRes OnSendNetMsg(INetMessage&);

public:	//	ExtraPanel
	void OnRenderExtra() override final
	{
		///Add input
		ImGui::Checkbox("Fake Query CVars", bEnabled.get());
	}
} static smac_anti_cvars;


HookRes SMAC_AntiCVars::OnSendNetMsg(INetMessage& msg)
{
	if (!bEnabled)
		return HookRes::Continue;

	if (msg.GetType() == clc_RespondCvarValue)
	{
		CLC_RespondCvarValue& cvar_respond = reinterpret_cast<CLC_RespondCvarValue&>(msg);
		const char* name = cvar_respond.m_szCvarName;
		auto i = m_CVarsList.find(name);

		if (i != m_CVarsList.end())
			cvar_respond.m_szCvarValue = i->second.c_str();
	}
	else if(msg.GetType() == clc_CmdKeyValues && MIKUDebug::m_bDebugging)
	{
		KeyValues* kv_msg = reinterpret_cast<CLC_CmdKeyValues&>(msg).m_pKeyValues;
		ParseKeyValues(kv_msg);
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

void ParseKeyValues(KeyValues* kv)
{
	MIKUDebug::LogDebug(Format("\n\nBEGIN KV: \"", kv->GetName(), "\""));

	std::vector<KeyValues*> kvs{ };
	{
		FOR_EACH_SUBKEY(kv, subkey)
			FOR_EACH_VALUE(subkey, block)
				if (std::find(kvs.begin(), kvs.end(), block) == kvs.end())
					kvs.push_back(block);
		FOR_EACH_VALUE(kv, block)
			if (std::find(kvs.begin(), kvs.end(), block) == kvs.end())
				kvs.push_back(block);
	}

	char dt = KeyValues::types_t::TYPE_NONE;
	const char* name = "";

	for (KeyValues* kv : kvs)
	{
		dt = kv->m_iDataType;
		name = kv->GetName();

		switch (dt)
		{
		
		case KeyValues::types_t::TYPE_STRING:
		{
			if (kv->m_sValue && *(kv->m_sValue))
			{
				MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - STRING: ", kv->m_sValue));
			}
			else
			{
				MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - STRING: <EMPTY>"));
			}
			break;
		}

		case KeyValues::types_t::TYPE_INT:
		{
			MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - INT: ", kv->m_iValue));
			break;
		}

		case KeyValues::types_t::TYPE_UINT64:
		{
			MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - DOUBLE: ", *reinterpret_cast<double*>(kv->m_sValue)));
			break;
		}

		case KeyValues::types_t::TYPE_FLOAT:
		{
			MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - FLOAT: ", kv->m_flValue));
			break;
		}
		case KeyValues::types_t::TYPE_COLOR:
		{
			MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - COLOR: { ", kv->m_Color[0], " ", kv->m_Color[1], " ", kv->m_Color[2], " ", kv->m_Color[3], " }"));
			break;
		}
		case KeyValues::types_t::TYPE_PTR:
		{
			MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - PTR: ", kv->m_pValue));
			break;
		}
		default:
		{
			MIKUDebug::LogDebug(Format("\tKey: \"", name, "\" - UNKNOWN"));
			break;
		}
		}
	}

	MIKUDebug::LogDebug("END KV");
}

HAT_COMMAND(revive_marker, "")
{
	KeyValues* kv = new KeyValues("MVM_Revive_Response");
	kv->SetInt("accepted", 1);
	engineclient->ServerCmdKeyValues(kv);
}