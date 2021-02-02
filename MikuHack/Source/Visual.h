#pragma once
#include "Main.h"
#include "../Helpers/VTable.h"

class VisualMenu : public MenuPanel
{
public:	// VisualMenu
	AutoBool bHideScope			{ "VisualMenu::bHideScope", false };
	AutoBool bCleanScreenshot	{ "VisualMenu::bCleanScreenshot", true };
	AutoBool bWhosWatchingMe	{ "VisualMenu::bWhosWatchingMe", true };
	AutoArray<int, 2> iWatchOffset	{ "VisualMenu::Offset" };

	VisualMenu()
	{
		constexpr int watchOffset[2]{ 12, 130 };
		iWatchOffset[0] = watchOffset[0];
		iWatchOffset[1] = watchOffset[1];

		IGlobalEvent::PaintTraverse::Hook::Register(std::bind(&VisualMenu::OnPaintTraverse, this));
	};

public:
	HookRes OnPaintTraverse();

public:	// MenuPanel
	void OnRender() override;
	void JsonCallback(Json::Value& json, bool read) override;
};

extern VisualMenu VMenu;