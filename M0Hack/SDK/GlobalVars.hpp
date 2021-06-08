#pragma once


class IValveGlobalVars
{
public:
	// Absolute time (per frame still - Use Plat_FloatTime() for a high precision real time 
	//  perf clock, but not that it doesn't obey host_timescale/host_framerate)
	float			RealTime;
	// Absolute frame counter
	int				FrameCount;
	// Non-paused frametime
	float			AbsFrameTime;

	// Current time 
	//
	// On the client, this (along with tickcount) takes a different meaning based on what
	// piece of code you're in:
	// 
	//   - While receiving network packets (like in PreDataUpdate/PostDataUpdate and proxies),
	//     this is set to the SERVER TICKCOUNT for that packet. There is no interval between
	//     the server ticks.
	//     [server_current_Tick * tick_interval]
	//
	//   - While rendering, this is the exact client clock 
	//     [client_current_tick * tick_interval + interpolation_amount]
	//
	//   - During prediction, this is based on the client's current tick:
	//     [client_current_tick * tick_interval]
	float			CurTime;

	// Time spent on last server or client frame (has nothing to do with think intervals)
	float			FrameTime;
	// current maxplayers setting
	int				MaxClients;

	// Simulation ticks
	int				TickCount;

	// Simulation tick interval
	float			IntervalPerTick;

	// interpolation amount ( client-only ) based on fraction of next tick which has elapsed
	float			InterpAmount;
	int				SimTicksThisFrame;

	int				NetProtocol;

	// current saverestore data
	void*			SaveRestoreData;

	// Set to true in client code.
	bool			IsClient;

	// 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
	int				TimestampNetworkingBase;
	// 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->tickcount to set the networking basis, prevents
	//  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting lockstepped on this)
	int				TimestampRandomizeWindow;
};

namespace Interfaces
{
	extern IValveGlobalVars* GlobalVars;
}