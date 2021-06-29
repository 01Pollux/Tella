#pragma once

#include <random>

namespace tella
{
	namespace random
	{
		using default_device = std::random_device;

		template<typename _Eng = default_device>
		[[nodiscard]] inline bool Bool(_Eng& seed)
		{
			std::bernoulli_distribution r(0.5);
			return r(seed);
		}

		[[nodiscard]] inline bool Bool()
		{
			default_device eng;
			return Bool(eng);
		}

		template<typename _Eng = default_device>
		[[nodiscard]] inline int Int(int min, int max, _Eng& seed)
		{
			std::uniform_int_distribution<int> r(min, max);
			return r(seed);
		}

		[[nodiscard]] inline int Int(int min, int max)
		{
			default_device eng;
			return Int(min, max, eng);
		}

		template<typename _Eng = default_device>
		[[nodiscard]] inline float Float(float min, float max, _Eng& seed)
		{
			std::uniform_real_distribution<float> r(min, max);
			return r(seed);
		}

		[[nodiscard]] inline float Float(float min, float max)
		{
			default_device eng;
			return Float(min, max, eng);
		}

		template<typename _Eng = default_device>
		[[nodiscard]] inline double Double(double min, double max, _Eng& seed)
		{
			std::uniform_real_distribution<double> r(min, max);
			return r(seed);

		}

		[[nodiscard]] inline double Double(double min, double max)
		{
			default_device eng;
			return Double(min, max, eng);
		}
	}
}
