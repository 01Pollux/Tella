#pragma once

class IHudElement;

class ITFHudManager
{
public:
	ITFHudManager();

	IHudElement*	operator[](size_t index) noexcept { return Huds[index]; }
	size_t			size() const noexcept { return Count; }

	IHudElement*		find(const char* name) noexcept;
	const IHudElement*	find(const char* name) const noexcept;

public:
	ITFHudManager(const ITFHudManager&) = default;	ITFHudManager& operator=(const ITFHudManager&) = default;
	ITFHudManager(ITFHudManager&&) = delete;		ITFHudManager& operator=(ITFHudManager&&) = delete;
	~ITFHudManager() = default;
	bool operator<=>(const ITFHudManager&) const = delete;

	auto begin() noexcept { return Huds; }
	const auto begin() const noexcept { return Huds; }
	auto end() noexcept { return Huds + Count; }
	const auto end() const noexcept { return Huds + Count; }

private:
	size_t			Count;
	IHudElement**	Huds;
};