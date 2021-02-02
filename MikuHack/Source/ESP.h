#pragma once

#include "Main.h"
#include "../Helpers/VTable.h"

struct ESPData;
class IClientShared;

class ESPMenu: public MenuPanel
{
public:

	struct ESPStruct
	{
		bool				m_bActive;

		int					m_iDrawColor[4];
		int					m_iFontColor[4];
		float				m_flMaxDist;

		enum ESPMode
		{
			BOX =		0b01,
			BOX3D =		0b10,
		};
		ESPMode				m_bitsDrawMode;

		bool				m_bDrawName;
		bool				m_bDrawDistance;
	};

	struct PESPStruct {
		ESPStruct base;

		bool m_bDrawClass;
		bool m_bDrawCond;
		bool m_bDrawUber;
		bool m_bIgnoreCloak;

		bool m_bDrawHealth;
		bool m_bDrawTeam;
	}; 
	PESPStruct player_esp[2]; 	//0: RED, 1: BLU

	struct BESPStruct {
		ESPStruct base;

		bool m_bDrawOwner;		//TODO!
		bool m_bDrawAmmo;
		bool m_bDrawLevel;
		bool m_bDrawState;

		bool m_bDrawHealth;
		bool m_bDrawTeam;

	};
	BESPStruct building_esp[2]; 	//0: RED, 1: BLU

	struct OESPStruct {
		ESPStruct base;

		bool m_bDrawPacks;
		bool m_bDrawRockets;
		bool m_bDrawStickies;
	};
	OESPStruct objects_esp;

public: //MenuPanel

	enum ESPType_t: uint
	{
		Player,
		Building,
		Objects,

		ESPMAX
	};

	void OnRender() override;
	void JsonCallback(Json::Value& json, bool read) override;

public:

	ESPMenu()
	{
		IGlobalEvent::PaintTraverse::Hook::Register(std::bind(&ESPMenu::OnPaintTraverse, this));
	};

	HookRes OnPaintTraverse();

private:

	void CollectEntities();
	void DrawEntities(ESPData& data);

	bool DrawBox(ESPData& data);
	bool IsActive() 
	{ 
		return  player_esp[0].base.m_bActive	||
				building_esp[0].base.m_bActive	||
				objects_esp.base.m_bActive		||
				player_esp[1].base.m_bActive	||
				building_esp[1].base.m_bActive	; 
	}
};

extern ESPMenu EMenu;