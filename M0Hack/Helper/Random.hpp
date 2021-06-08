#pragma once

#include <random>

namespace Random
{
	using RandomDevice = std::random_device;

	template<typename RandomSeed = RandomDevice>
	[[nodiscard]] inline bool Bool(RandomSeed& seed)
	{
		std::bernoulli_distribution r(0.5);
		return r(seed);
	}

	[[nodiscard]] inline bool Bool()
	{
		RandomDevice eng;
		return Bool(eng);
	}

	template<typename RandomSeed = RandomDevice>
	[[nodiscard]] inline int Int(int min, int max, RandomSeed& seed)
	{
		std::uniform_int_distribution<int> r(min, max);
		return r(seed);
	}

	[[nodiscard]] inline int Int(int min, int max)
	{
		RandomDevice eng;
		return Int(min, max, eng);
	}

	template<typename RandomSeed = RandomDevice>
	[[nodiscard]] inline float Float(float min, float max, RandomSeed& seed)
	{
		std::uniform_real_distribution<float> r(min, max);
		return r(seed);
	}

	[[nodiscard]] inline float Float(float min, float max)
	{
		RandomDevice eng;
		return Float(min, max, eng);
	}

	template<typename RandomSeed = RandomDevice>
	[[nodiscard]] inline double Double(double min, double max, RandomSeed& seed)
	{
		std::uniform_real_distribution<double> r(min, max);
		return r(seed);

	}

	[[nodiscard]] inline double Double(double min, double max)
	{
		RandomDevice eng;
		return Double(min, max, eng);
	}
}
