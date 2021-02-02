#pragma once
#include "Main.h"

class AimbotMenu : public MenuPanel
{
public:
	AimbotMenu();
	HookRes OnCreateMove(bool& ret);

public:	// MenuPanel

	void OnRender() override;
	void JsonCallback(Json::Value& json, bool read) override;
};
