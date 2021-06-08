#pragma once

#include <format>
#include <wchar.h>
#include <chrono>

using namespace std::string_view_literals;
using namespace std::string_literals;


template<typename _CTy>
class IFormatterT
{
public:
	using std_string_view_type = std::basic_string_view<_CTy>;
	using std_string_type = std::conditional_t<std::is_same_v<_CTy, wchar_t>, std::wstring, std::conditional_t<std::is_same_v<_CTy, char>, std::string, void>>;
	static_assert(!std::is_same_v<std_string_type, void>);

	IFormatterT(const IFormatterT&) = default;	IFormatterT& operator=(const IFormatterT&) = default;
	IFormatterT(IFormatterT&&) = default;		IFormatterT& operator=(IFormatterT&&) = default;

	template<typename... _Args>
	IFormatterT(const std_string_view_type& fmt, _Args&&... args) { Buffer = std::format(fmt, std::forward<_Args>(args)...); }
	IFormatterT(size_t size) noexcept { reserve(size); }
	IFormatterT() = default;

	void reserve(size_t size)			noexcept		{ Buffer.reserve(size); }
	void resize(size_t size)			noexcept		{ Buffer.resize(size); }

	_NODISCARD size_t capacity()		const noexcept	{ return Buffer.capacity(); }
	_NODISCARD size_t size()			const noexcept	{ return Buffer.size(); }

	_NODISCARD bool empty()				const noexcept	{ return Buffer.empty(); }
	_NODISCARD auto begin()				const noexcept	{ return Buffer.begin(); }
	_NODISCARD auto begin()				noexcept		{ return Buffer.begin(); }
	_NODISCARD auto end()				const noexcept	{ return Buffer.end(); }
	_NODISCARD auto end()				noexcept		{ return Buffer.end(); }

	void clear()						noexcept		{ Buffer.clear(); }
	_NODISCARD const _CTy* c_str()		const noexcept	{ return Buffer.c_str(); }
	_NODISCARD _CTy* data()				noexcept		{ return Buffer.data(); }

	operator const _CTy*()				const noexcept	{ return c_str(); };
	operator const std_string_type&()	const noexcept	{ return Buffer; };
	operator std_string_type&()			noexcept		{ return Buffer; };
	_NODISCARD std_string_type str()	const noexcept	{ return Buffer; };

	template<typename... _Args>
	_NODISCARD IFormatterT& operator()(const std_string_view_type& fmt, _Args&&... args)
	{
		Buffer = std::format(fmt, std::forward<_Args>(args)...);
		return *this;
	}

private:
	std_string_type Buffer;
};

using IFormatter	= IFormatterT<char>;
using IFormatterW	= IFormatterT<wchar_t>;


template<typename _CTy, size_t _Size>
class IFormatterNT
{
public:
	using std_string_view_type = std::basic_string_view<_CTy>;
	using std_string_type = std::conditional_t<std::is_same_v<_CTy, wchar_t>, std::wstring, std::conditional_t<std::is_same_v<_CTy, char>, std::string, void>>;
	static_assert(!std::is_same_v<std_string_type, void>);

	IFormatterNT(const IFormatterNT&) = delete;	IFormatterNT& operator=(const IFormatterNT&) = delete;
	IFormatterNT(IFormatterNT&&) = delete;		IFormatterNT& operator=(IFormatterNT&&) = delete;

	template<typename... _Args>
	IFormatterNT(const std_string_view_type& fmt, _Args&&... args) { Buffer = std::format(fmt, std::forward<_Args>(args)...); }
	IFormatterNT() = default;

	void resize(size_t size)		noexcept		{ Length = std::clamp(0U, size, max_size()); Buffer[Length] = 0; }
	_NODISCARD size_t max_size()	const noexcept	{ return _Size; }
	_NODISCARD size_t size()		const noexcept	{ return Length; }
	
	_NODISCARD bool empty()			const noexcept	{ return size() != 0; }
	_NODISCARD auto begin()			const noexcept	{ return std::begin(Buffer); }
	_NODISCARD auto begin()			noexcept		{ return std::begin(Buffer); }
	_NODISCARD auto end()			const noexcept	{ return std::end(Buffer); }
	_NODISCARD auto end()			noexcept		{ return std::end(Buffer); }

	void clear()					noexcept		{ resize(0); }
	_NODISCARD const _CTy* c_str()	const noexcept	{ return Buffer; }
	_NODISCARD _CTy* data()			noexcept		{ return Buffer; }

	operator const _CTy*()			const noexcept	{ return c_str(); };
	_NODISCARD std_string_type str()const noexcept	{ return Buffer; };

	template<typename... _Args>
	_NODISCARD IFormatterNT& operator()(const std_string_view_type& fmt, _Args&&... args)
	{
		Length = std::format_to_n(Buffer, _Size, fmt, std::forward<_Args>(args)...).size;
		return *this;
	}

private:
	_CTy Buffer[_Size];
	size_t Length;
};

template<size_t _Size> using IFormatterN = IFormatterNT<char, _Size>;
template<size_t _Size> using IFormatterNW = IFormatterNT<wchar_t, _Size>;


template<typename _CTy>
class IFormatterTSV
{
public:
	using std_string_view_type = std::basic_string_view<_CTy>;
	using std_string_type = std::conditional_t<std::is_same_v<_CTy, wchar_t>, std::wstring, std::conditional_t<std::is_same_v<_CTy, char>, std::string, void>>;
	static_assert(!std::is_same_v<std_string_type, void>);

	IFormatterTSV(const IFormatterTSV&) = default;	IFormatterTSV& operator=(const IFormatterTSV&) = default;
	IFormatterTSV(IFormatterTSV&&) = delete;		IFormatterTSV& operator=(IFormatterTSV&&) = delete;

	IFormatterTSV(const std_string_view_type& fmt) noexcept : Template(fmt) { };
	
	_NODISCARD size_t size()						const noexcept { return Template.size(); }
	_NODISCARD const _CTy* c_str()					const noexcept { return Template.data(); }

	template<typename... _Args>
	_NODISCARD std_string_type operator()(_Args&&... args) const
	{
		return std::format(Template, std::forward<_Args>(args)...);
	}

private:
	std_string_view_type Template;
};

using IFormatterSV = IFormatterTSV<char>;
using IFormatterWSV = IFormatterTSV<wchar_t>;


template<typename _To, typename _From>
_NODISCARD constexpr _To StringTransform(const _From& str) noexcept
{
	if constexpr (std::is_same_v<_To, _From>)
		return str;
	else if constexpr (std::is_pointer_v<_From>)
		return StringTransform<_To>(std::basic_string_view<std::remove_pointer_t<_From>>(str));
	else
	{
		if (str.empty())
			return { };
		else
		{
			///  from wstring to string, u32string to u8string, etc...
			if constexpr (sizeof(_From::value_type) > sizeof(_To::value_type))
			{
				_To buf(str.size(), 0);
				std::transform(std::begin(str), std::end(str), std::begin(buf), [](const _From::value_type c) { return static_cast<_To::value_type>(c); });
				return buf;
			}
			else return { std::begin(str), std::end(str) };
		}
	}
}

template<typename _To, typename _From, size_t _Size>
_NODISCARD constexpr _To StringTransform(const _From (&str)[_Size]) noexcept
{
	if constexpr (std::is_same_v<_To::value_type, _From>)
		return str;

	if (_Size < 1)
		return { };
	else return { std::begin(str), std::end(str) };
}

_NODISCARD inline std::string FormatTime(const std::string_view& fmt)
{
	auto now = std::chrono::system_clock::now();
	return std::format(fmt, now);
}

void FixEscapeCharacters(std::string& str);