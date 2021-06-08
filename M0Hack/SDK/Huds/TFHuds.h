#pragma once

class IHudElement;

class ITFHudManager
{
public:
	ITFHudManager();

	IHudElement*	operator[](size_t index) noexcept { return huds[index]; }
	size_t			size() const noexcept { return count; }

	IHudElement*		find(const char* name) noexcept;
	const IHudElement*	find(const char* name) const noexcept;

public:
	ITFHudManager(const ITFHudManager&) = default;	ITFHudManager& operator=(const ITFHudManager&) = default;
	ITFHudManager(ITFHudManager&&) = delete;		ITFHudManager& operator=(ITFHudManager&&) = delete;
	~ITFHudManager() = default;
	bool operator<=>(const ITFHudManager&) const = delete;

	auto begin() noexcept { return huds; }
	const auto begin() const noexcept { return huds; }
	auto end() noexcept { return huds + count; }
	const auto end() const noexcept { return huds + count; }

private:
	size_t count;
	IHudElement** huds;
};