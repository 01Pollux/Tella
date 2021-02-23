#pragma once

#include "../Helpers/AutoList.h"
#include "../ImGui/imgui_helper.h"

#include "../Helpers/json.h"
#include "../Helpers/AutoItem.h"

class MenuPanel: public IAutoList<MenuPanel>
{
public:
	virtual void OnRender() { }
	virtual void OnMenuInit() { }
	virtual void JsonCallback(Json::Value& json, bool read) { };
	virtual void OnRenderExtra() { };
};

class MainMenu: public MenuPanel
{
public:
	float m_flColor[4];
	bool m_bIsActive;

public:
	static void RenderMenuBar();
	static void DoRender();
};
extern MainMenu Mmain;

class ExtraPanel : public MenuPanel
{
public:
	void OnMenuInit() final { };
	void OnRender() final { };

	void JsonCallback(Json::Value& json, bool read) override { };
	void OnRenderExtra() override { };
};



#define PROCESS_JSON_READ(CONFIG, MEMBER, TYPE, VAR) \
		if ( ## CONFIG ## .isMember(## MEMBER ## ))		VAR = CONFIG ## [MEMBER].as##TYPE##()

#define PROCESS_JSON_WRITE(CONFIG, MEMBER, VAR) \
		CONFIG ## [MEMBER] = VAR;

#define PROCESS_JSON_READ_COLOR(CONFIG, MEMBER, TYPE, VAR) \
		{ \
			Json::Value& tmp = CONFIG [ MEMBER ]; \
			if (tmp.isMember("r")) VAR [0] = tmp["r"].as##TYPE##(); \
			if (tmp.isMember("g")) VAR [1] = tmp["g"].as##TYPE##(); \
			if (tmp.isMember("b")) VAR [2] = tmp["b"].as##TYPE##(); \
			if (tmp.isMember("a")) VAR [3] = tmp["a"].as##TYPE##(); \
		}

#define PROCESS_JSON_WRITE_COLOR(CONFIG, MEMBER, VAR) \
		{ \
			Json::Value& tmp = CONFIG [ MEMBER ]; \
			tmp["r"] = VAR [0]; \
			tmp["g"] = VAR [1]; \
			tmp["b"] = VAR [2]; \
			tmp["a"] = VAR [3]; \
		}