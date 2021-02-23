#pragma once

#include "Main.h"
#include "../Helpers/Commons.h"
#include "../GlobalHook/vhook.h"
#include "../GlobalHook/load_routine.h"

#include <memory>
#include <array>

constexpr size_t SizeOfBacktrack = 67U;

struct IBackTrackData
{
	int			tick{ };
	float		simulation_time;

	Vector		origin;
	QAngle		angles;

	constexpr operator bool() const noexcept
	{
		return tick != 0;
	}
};


class IBackTrackInfo
{
	std::unique_ptr<IBackTrackData[]> data{ };

public:
	using FilterCallback = std::function<void(int, const IBackTrackData&)>;

	void init()
	{
		data = std::make_unique<IBackTrackData[]>(SizeOfBacktrack);
	}

	IBackTrackData& operator[](size_t pos) const
	{
		return data[pos];
	}

	operator bool() const noexcept
	{
		return data != nullptr;
	}

	void reset() noexcept
	{
		data = nullptr;
	}
};


class CUserCmd;
class IBackTrackHack : public ExtraPanel, public IMainRoutine
{
	AutoBool bEnable		{ "BackTrack::bEnabled", false };
	AutoBool should_reset	{ "BackTrack::Reset", false };
	AutoFloat flTrackFOV	{ "BackTrack::flTrackFOV", 90.f };
	IBackTrackInfo backtrack[MAX_PLAYERS]{};

public:	//	IBackTrackHack
	HookRes OnCreateMove(CUserCmd*);

public:	//	ExtraPanel
	void OnRenderExtra() override;
//	void JsonCallback(Json::Value& json, bool read) override;

public: //	IMainRoutine
	void OnLoadDLL() override;
};
