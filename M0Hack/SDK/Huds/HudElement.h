#pragma once

#include "TFHuds.h"
#include "VGUI.hpp"

class IHudElement
{
public:
	/// 0
	virtual ~IHudElement() = default;

	/// 1
	virtual void DummyFireGameEvent(void*) abstract;
	/// 2
	virtual void Init() abstract;
	///	3
	virtual void VidInit() abstract;
	///	4
	virtual void DummyLevelInit() abstract;
	///	5
	virtual void DummyLevelShutdown() abstract;
	///	6
	virtual void Reset() abstract;
	///	7
	virtual void ProcessInput() abstract;
	///	8
	virtual const char* GetName() abstract;
	///	9
	virtual bool ShouldDraw() abstract;
	///	10
	virtual bool IsActive() abstract;
	///	11
	virtual void SetActive(bool state) abstract;


public:
	IHudElement() = delete;	
	IHudElement(const IHudElement&) = delete;	IHudElement& operator=(const IHudElement&) = delete;
	IHudElement(IHudElement&&) = delete;		IHudElement& operator=(const IHudElement&&) = delete;
};

VGUI::VPANEL FindPanelByName(const char* name);
