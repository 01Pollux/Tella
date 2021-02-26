#pragma once

#include <string>
#include "../fmt/format.h"
#include "../fmt/chrono.h"

class IFormatter
{
public:
	IFormatter() = default;
	IFormatter(const IFormatter&) = delete; IFormatter& operator=(const IFormatter&) = delete;
	IFormatter(IFormatter&&) = delete; IFormatter& operator=(IFormatter&&) = delete;

	IFormatter(std::string_view str) : template_string(str) { };

	void format(std::string_view str) { template_string = str; }

	template<typename... Args>
	std::string operator()(Args&&... args)
	{
		return fmt::format(template_string, std::forward<Args>(args)...);
	}

private:
	std::string_view template_string;
};

class INFormatter
{
public:
	INFormatter() = default;
	INFormatter(const INFormatter&) = delete; INFormatter& operator=(const INFormatter&) = delete;
	INFormatter(INFormatter&&) = delete; INFormatter& operator=(INFormatter&&) = delete;

	INFormatter(std::string_view str, size_t size) : template_string(str), buffer_size(size)
	{
		buffer = std::make_unique<char[]>(size + 1);
	};

	void format(std::string_view str) noexcept { template_string = str; }

	size_t size() const noexcept { return buffer_size; }

	void resize(size_t size)
	{
		if (buffer_size != size)
		{
			buffer = std::make_unique<char[]>(size + 1);
			buffer_size = size;
		}
	}

	template<typename... Args>
	auto operator()(Args&&... args)
	{
		fmt::format_to_n(buffer.get(), buffer_size, template_string, std::forward<Args>(args)...);
		return buffer.get();
	}

private:
	std::string_view template_string;
	size_t buffer_size;
	std::unique_ptr<char[]> buffer;
};

class IFastFormatter
{
public:
	template<typename... Args>
	IFastFormatter(std::string_view str, Args&&... args)
	{
		IFormatter fmt(str);
		buffer = std::move(fmt(args...));
	}

	std::string& str() noexcept
	{
		return buffer;
	}

private:
	std::string buffer;
};

class IBufferFormatter
{
public:
	IBufferFormatter() = default;
	IBufferFormatter(const IBufferFormatter&) = delete; IBufferFormatter& operator=(const IBufferFormatter&) = delete;
	IBufferFormatter(IBufferFormatter&&) = delete; IBufferFormatter& operator=(IBufferFormatter&&) = delete;

	template<typename... Args>
	void cat(std::string_view template_string, Args&&... args)
	{
		buffer += fmt::format(template_string, args...);
	}

	void clear()
	{
		buffer.clear();
	}

	void reserve(size_t size)
	{
		buffer.reserve(size);
	}

	std::string& operator*() noexcept
	{
		return buffer;
	}

private:
	std::string buffer;
};