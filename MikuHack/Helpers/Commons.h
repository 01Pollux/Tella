#pragma once

#include "../Source/Debug.h"

#include "Library.h"
#include "Timer.h"
#include "NetVars.h"
#include "Offsets.h"

#include "../Interfaces/CBaseEntity.h"
#include "../Interfaces/HatCommand.h"
#include "../Interfaces/Main.h"

#include <random>

template<typename T, int size>
constexpr int SizeOfArray(T(&)[size])
{
	return size;
}

namespace Random
{
	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] bool Bool()
	{
		std::uniform_int_distribution<int> r(0, 1);
		RandomEngine _engine;
		return r(_engine) ? true:false;
	}

	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] int Int(int min, int max)
	{
		std::uniform_int_distribution<int> r(min, max);
		RandomEngine _engine;
		return r(_engine);
	}

	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] float Float(float min, float max)
	{
		std::uniform_real_distribution<float> r(min, max);
		RandomEngine _engine;
		return r(_engine);
	}

	template<typename RandomEngine = std::default_random_engine>
	[[nodiscard]] double Double(double min, double max)
	{
		std::uniform_real_distribution<Double> r(min, max);
		RandomEngine _engine;
		return r(_engine);
	}
}

