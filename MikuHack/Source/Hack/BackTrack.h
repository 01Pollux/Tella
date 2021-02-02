#pragma once

/*#include "../Helpers/sdk.h"
#include "../Helpers/Commons.h"
#include "../Helpers/VTable.h"

#include <memory>
#include <array>

constexpr size_t SIZE_OF_BACKTRACK = 67U;

struct IBackTrackData
{
	int			tick{ };
	float		simtime;

	Vector		origin;
	QAngle		angles;

	bool valid()
	{
		return tick != 0;
	}
};


class IBackTrackInfo
{
	std::unique_ptr<IBackTrackData[]> data;

public:
	using FilterCallback = std::function<void(int, const IBackTrackData&)>;

	void init()
	{
		data = std::make_unique<IBackTrackData[]>(SIZE_OF_BACKTRACK);
	}

	IBackTrackData& operator[](size_t pos) const
	{
		return data[pos];
	}

	operator bool() const noexcept
	{
		return data != nullptr;
	}

	void reset()
	{
		data = nullptr;
	}
};



class IBackTrackHack	//: public MenuPanel
{
public: //InventoryHack
	AutoBool bEnable	{ "BackTrack::bEnabled", true };
	AutoFloat flTrackFOV{ "BackTrack::flTrackFOV", 90.f };
	IBackTrackInfo backtrack[MAX_PLAYERS]{};

public:
	IBackTrackHack()
	{
//		IGlobalEvent::CreateMove::Hook::Register(std::bind(&IBackTrackHack::OnPaintTraverse, this));
		IGlobalEvent::LoadDLL::Hook::Register(std::bind(&IBackTrackHack::OnLoad, this));
		IGlobalEvent::UnloadDLL::Hook::Register(std::bind(&IBackTrackHack::OnUnload, this));
	};

//	HookRes OnCreateMove(bool&);
	HookRes OnLoad();
	HookRes OnUnload();

public:	//MenuPanel
//	void OnRender() override;
//	void JsonCallback(Json::Value& json, bool read) override;
};

*/