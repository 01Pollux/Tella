#pragma once



class IHudElement;

class ITFHudManager
{
	int count;
	IHudElement** huds;

public:
	ITFHudManager();
	IHudElement*	operator[](size_t index) { return huds[index]; }
	int				HudCount() { return count; }

public:
	ITFHudManager(const ITFHudManager&) = default;
	ITFHudManager& operator=(const ITFHudManager&) = default;
	ITFHudManager(ITFHudManager&&) = delete;
	ITFHudManager& operator=(ITFHudManager&&) = delete;

	bool operator==(const ITFHudManager&) = delete;
};