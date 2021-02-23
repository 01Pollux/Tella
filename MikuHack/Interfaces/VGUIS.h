#pragma once

#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <inputsystem/iinputsystem.h>


extern vgui::IPanel* panel;
extern vgui::ISurface* surface;


struct PanelIdAndName
{
	uint id;
	const char* name;
};

extern uint PID_FocusOverlay;
extern uint PID_HudScope;