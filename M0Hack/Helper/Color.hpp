#pragma once

#include "ImGui/imgui_helper.h"
#include "Config.hpp"

namespace tella
{
	namespace color
	{
		constexpr char8_t u8alpha = 255U;
		constexpr float falpha = 1.f;
		constexpr int ialpha = 255;

		template<typename ColorType, ColorType DefAlpha>
		struct rgba
		{
			using type = ColorType;

			constexpr rgba()						noexcept = default;
			rgba(const rgba&)						noexcept = default; rgba(rgba&&) noexcept = default;
			rgba& operator=(const rgba&)			noexcept = default;	rgba& operator=(rgba&&) noexcept = default;

			constexpr rgba(type r, type g, type b, type a = DefAlpha)		noexcept : r(r), g(g), b(b), a(a) { }
			constexpr rgba(const std::array<type, 4>& color)				noexcept : r(color[0]), g(color[1]), b(color[2]), a(color[3]) { }
			constexpr rgba(const std::array<type, 3>& color)				noexcept : r(color[0]), g(color[1]), b(color[2]), a(DefAlpha) { }
			constexpr rgba(const type(&color)[4])							noexcept : r(color[0]), g(color[1]), b(color[2]), a(color[3]) { }
			constexpr rgba(const type(&color)[3])							noexcept : r(color[0]), g(color[1]), b(color[2]), a(DefAlpha) { }

			_NODISCARD type* data()													noexcept { return reinterpret_cast<type*>(this); }
			_NODISCARD type& operator[](size_t i)										noexcept { return data()[i]; }
			_NODISCARD const type* data()										  const noexcept { return reinterpret_cast<const type*>(this); }
			_NODISCARD const type& operator[](size_t i)						  const noexcept { return data()[i]; }

			type r{ }, g{ }, b{ }, a{ };

			_NODISCARD constexpr auto operator<=>(const rgba& o)		const noexcept = default;

			_NODISCARD constexpr rgba operator+(const rgba& o)			const noexcept { return { r + o.r, g + o.g, b + o.b, DefAlpha }; }
			_NODISCARD constexpr rgba operator+=(const rgba& o)			noexcept { r += o.r; g += o.g; b += o.b;  return *this; }

			_NODISCARD constexpr rgba operator-(const rgba& o)			const noexcept { return { r - o.r, g - o.g, b - o.b, DefAlpha }; }
			constexpr rgba operator-=(const rgba& o)					noexcept { r -= o.r; g -= o.g; b -= o.b;  return *this; }

			_NODISCARD constexpr rgba operator/(const float p)			const noexcept { return { to_type(r / p), to_type(g / p), to_type(b / p), DefAlpha }; }
			constexpr rgba operator/=(const float p)					noexcept { r = to_type(r / p); g = to_type(g / p); b = to_type(b / p);  return *this; }

			_NODISCARD constexpr rgba operator*(const float p)			const noexcept { return { to_type(r * p), to_type(g * p), to_type(b * p), DefAlpha }; }
			constexpr rgba operator*=(const float p)					noexcept { r = to_type(r * p); g = to_type(g * p); b = to_type(b * p);  return *this; }

			constexpr operator std::array<type, 4>()					const noexcept { return { r, g, b, a }; }

			void clamp()
			{
				for (size_t i = 0; i < sizeof(*this); i++)
				{
					type& cur = this->operator[](i);

					if constexpr (std::is_same_v<type, char8_t>)
					{
						if (cur > 255U)
							cur = 255U;
					}
					else if constexpr (std::is_same_v<type, float> || std::is_same_v<type, double>)
					{
						if (cur > 1.0)
							cur = 1.0;
						else if (cur < 0.0)
							cur = 0.0;
					}
				}
			}

		private:
			template<typename AnyType>
			constexpr type to_type(const AnyType& in) const noexcept
			{
				return static_cast<type>(in);
			}
		};

		using u8rgba = rgba<char8_t, u8alpha>;
		using frgba = rgba<float, falpha>;

		template<typename ColorType, ColorType DefAlpha>
		_NODISCARD constexpr u8rgba to_u8rgba(const rgba<ColorType, DefAlpha>& o)
		{
			if constexpr (std::is_same_v<ColorType, u8rgba::type>)
				return o;
			else return { static_cast<u8rgba::type>(o.r * 255), static_cast<u8rgba::type>(o.g * 255), static_cast<u8rgba::type>(o.b * 255), static_cast<u8rgba::type>(o.a * 255) };
		}

		template<typename ColorType, ColorType DefAlpha>
		_NODISCARD constexpr frgba to_frgba(rgba<ColorType, DefAlpha> o)
		{
			if constexpr (std::is_same_v<ColorType, frgba::type>)
				return o;
			else return { o.r / 255.f, o.g / 255.f, o.b / 255.f, o.a / 255.f };
		}

		namespace names
		{
			constexpr u8rgba black{ 0, 0, 0, 255 };
			constexpr u8rgba white{ 255, 255, 255, 255 };
			constexpr u8rgba grey{ 255, 255, 255, 125 };

			constexpr u8rgba red{ 255, 0, 0, 255 };
			constexpr u8rgba green{ 0, 255, 0, 255 };
			constexpr u8rgba blue{ 0, 0, 255, 255 };

			constexpr u8rgba violet{ 148, 0, 211, 255 };
			constexpr u8rgba indigo{ 75, 0, 130, 255 };

			constexpr u8rgba fuschia{ 255, 0, 255, 255 };
			constexpr u8rgba pink{ 228, 128, 255, 255 };

			constexpr u8rgba cyan{ 0, 231, 231, 255 };
			constexpr u8rgba teal{ 12, 165, 197, 255 };
			constexpr u8rgba turquoise{ 12, 165, 197, 255 };
			constexpr u8rgba lime{ 0, 255, 213, 255 };
		}
	}

	namespace config
	{
		template<bool HasNotifier>
		class _Color_Wrapper : public custom_array<char8_t, 4, HasNotifier>
		{
			using color_type = color::u8rgba;
			using base_type = custom_array<color_type::type, 4, HasNotifier>;
			TCONFIG_INHERIT_FROM(_Color_Wrapper, color_type, base_type);

		public:
			using array_type = std::array<color_type::type, 4>;
			using base_type::get;

			_NODISCARD color_type::type& r() noexcept { return this->get()[0]; }
			_NODISCARD color_type::type& g() noexcept { return this->get()[1]; }
			_NODISCARD color_type::type& b() noexcept { return this->get()[2]; }
			_NODISCARD color_type::type& a() noexcept { return this->get()[3]; }

			_NODISCARD color_type::type r() const noexcept { return this->get()[0]; }
			_NODISCARD color_type::type g() const noexcept { return this->get()[1]; }
			_NODISCARD color_type::type b() const noexcept { return this->get()[2]; }
			_NODISCARD color_type::type a() const noexcept { return this->get()[3]; }

			_NODISCARD constexpr color_type::type def_alpha() const noexcept { return color::u8alpha; }
			operator color_type() const noexcept { return color_type(this->get()); }

		protected:
			void _read(const Json::Value& inc) final
			{
				r() = static_cast<color_type::type>(inc["red"].asInt());
				g() = static_cast<color_type::type>(inc["green"].asInt());
				b() = static_cast<color_type::type>(inc["blue"].asInt());
				a() = static_cast<color_type::type>(inc["alpha"].asInt());
			}

			void _write(Json::Value& out) const final
			{
				out["red"] = r();
				out["green"] = g();
				out["blue"] = b();
				out["alpha"] = a();
			}
		};

		using U8Color = _Color_Wrapper<false>;
		using U8ColorN = _Color_Wrapper<true>;
	}
}