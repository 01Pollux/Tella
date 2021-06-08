#pragma once

#include <chrono>
#include <functional>

#include "EnumClass.hpp"
#include "Config.hpp"

using namespace std::chrono_literals;

enum class TimerFlags: char8_t
{
	Empty,
	AutoRepeat,
	ExecuteOnMapEnd
};

ECLASS_BITWISE_OPERATORS(TimerFlags);

using TimerRawData = void*;
using TimerCallbackFn = std::function<void(TimerRawData)>;
using TimerID = std::chrono::system_clock::rep;

class IHRTimer
{
	using HRC = std::chrono::steady_clock;
public:
	IHRTimer() noexcept
	{
		update();
	}

	void update() const
	{
		last_time = HRC::now();
	}

	_NODISCARD bool has_elapsed(std::chrono::milliseconds ms) const noexcept
	{
		return (HRC::now() - last_time) >= ms;
	}

	_NODISCARD bool trigger_if_elapsed(std::chrono::milliseconds ms) noexcept
	{
		if (has_elapsed(ms))
		{
			update();
			return true;
		}
		return false;
	}

protected:
	mutable HRC::time_point last_time;
};

class IUTimer : public IHRTimer
{
	using HRC = std::chrono::steady_clock;
public:
	IUTimer() noexcept : id(last_time.time_since_epoch().count()) { }

	_NODISCARD constexpr TimerID seed() const noexcept
	{
		return id;
	}

	_NODISCARD bool operator==(const IUTimer& other) const noexcept
	{
		return other.seed() == seed();
	}

	_NODISCARD bool operator==(const TimerID& other) const noexcept
	{
		return other == seed();
	}

private:
	TimerID id;
};

class ITimer
{
public:
	void update();
	
	_NODISCARD bool has_elapsed(float sec) const noexcept;

	_NODISCARD bool trigger_if_elapsed(float sec) noexcept
	{
		if (has_elapsed(sec))
		{
			update();
			return true;
		}
		return false;
	}

private:
	float last_time{ };
};

namespace ITimerSys
{
	const TimerID CreateFuture(std::chrono::milliseconds time, TimerFlags flags, const TimerCallbackFn& callback, TimerRawData data);

	void RequestFrame(TimerCallbackFn callback, void* data);

	void DeleteFuture(const TimerID& timer, bool execute = false);

	void RewindBack(const TimerID& timer);

	void ExecuteFrame();

	void RunOnLevelShutdown();

	namespace detail
	{
		template<class>
		struct is_std_chrono_duration
		{
			static constexpr bool value = false;
		};

		template<class Rep, class Period>
		struct is_std_chrono_duration<std::chrono::duration<Rep, Period>>
		{
			static constexpr bool value = true;
		};

		template<class... Args>
		inline constexpr bool is_std_chrono_duration_v = is_std_chrono_duration<Args...>::value;
	}
}

/**
 * Repeatable timer with changeable duration time
 */
template<typename ITimerW, typename TimeUnit>
class IAutoRTTimer
{
public:
	IAutoRTTimer() = default;

	IAutoRTTimer(TimeUnit tu) noexcept : tu(tu) { };

	void set_time(TimeUnit tu) noexcept
	{
		this->tu = tu;
	}

	_NODISCARD const TimeUnit& get_time() const noexcept
	{
		return tu;
	}

	_NODISCARD const ITimerW& get_timer() const noexcept
	{
		return timer;
	}

	void update() noexcept
	{
		timer.update();
	}

	_NODISCARD bool has_elapsed() const noexcept
	{
		return timer.has_elapsed(tu);
	}

	_NODISCARD bool trigger_if_elapsed() noexcept
	{
		return timer.trigger_if_elapsed(tu);
	}

private:
	ITimerW timer;
	TimeUnit tu{ };
};

/**
 * Repeatable timer with compile duration time
 */
template<typename ITimerW, float Dur>
class IAutoCTimer
{
public:
	_NODISCARD constexpr float get_time() const noexcept
	{
		return Dur;
	}

	_NODISCARD const ITimerW& get_timer() const noexcept
	{
		return timer;
	}

	void update() noexcept
	{
		timer.update();
	}

	_NODISCARD bool has_elapsed() const noexcept
	{
		return timer.has_elapsed(Dur);
	}

	_NODISCARD bool trigger_if_elapsed() noexcept
	{
		return timer.trigger_if_elapsed(Dur);
	}

private:
	ITimerW timer;
};

/**
 * Repeatable (high resolution) timer with compile duration time
 */
template<typename ITimerW, int Dur, class TimeUnit = std::chrono::milliseconds>
class AutoTimerUnk
{
	static_assert(ITimerSys::detail::is_std_chrono_duration_v<TimeUnit>, "TimeUnit must be a std::chrono::duration");
public:
	_NODISCARD constexpr TimeUnit get_time() const noexcept
	{
		return std::chrono::seconds(Dur);
	}

	_NODISCARD const ITimerW& get_timer() const noexcept
	{
		return timer;
	}

	void update() noexcept
	{
		timer.update();
	}

	_NODISCARD bool has_elapsed() const noexcept
	{
		return timer.has_elapsed(Dur);
	}

	_NODISCARD bool trigger_if_elapsed() noexcept
	{
		return timer.trigger_if_elapsed(Dur);
	}

private:
	ITimerW timer;
};



template<int Dur, class TimeUnit = std::chrono::milliseconds> 
using AutoHRTimer = AutoTimerUnk<IHRTimer, Dur, TimeUnit>;

template<int Dur, class TimeUnit = std::chrono::milliseconds>
using AutoUTimer = AutoTimerUnk<IUTimer, Dur, TimeUnit>;

using AutoRTTimer = IAutoRTTimer<ITimer, float>;

template<float Dur>
using AutoCTimer = IAutoCTimer<ITimer, Dur>;


M0CONFIG_BEGIN;

class RTTimer final : public Custom<AutoRTTimer, false>
{
	M0CONFIG_INHERIT_FROM(RTTimer, AutoRTTimer, Custom < AutoRTTimer, false>);

public:
	using M0Var_Internal<AutoRTTimer>::get;
	using M0Var_Internal<AutoRTTimer>::operator=;

	void set_time(float tu) noexcept
	{
		get().set_time(tu);
	}

	_NODISCARD const float& get_time() const noexcept
	{
		return get().get_time();
	}

	_NODISCARD const ITimer& get_timer() const noexcept
	{
		return get().get_timer();
	}

	void update() noexcept
	{
		get().update();
	}

	_NODISCARD bool has_elapsed() const noexcept
	{
		return get().has_elapsed();
	}

	_NODISCARD bool trigger_if_elapsed() noexcept
	{
		return get().trigger_if_elapsed();
	}

protected:
	void _write(Json::Value& out) const final
	{
		out = get_time();
	}

	void _read(const Json::Value& inc) final
	{
		set_time(inc.asFloat());
	}
};

M0CONFIG_END;