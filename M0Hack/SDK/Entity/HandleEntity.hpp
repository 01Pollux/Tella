#pragma once

#include "Const.hpp"

class IHandleEntity;

class IBaseHandle
{
public:
	IBaseHandle() = default;

	IBaseHandle(const IBaseHandle&) = default;
	IBaseHandle& operator=(const IBaseHandle&) = default;
	IBaseHandle(IBaseHandle&&) = default;
	IBaseHandle& operator=(IBaseHandle&&) = default;

	IBaseHandle(unsigned long value) noexcept : Index(value) { }
	IBaseHandle(int entry, int serial) noexcept : Index(entry | serial << NUM_ENT_ENTRY_BITS) { }
	
	int GetEntryIndex() const noexcept { return Index & ENT_ENTRY_MASK; }
	int GetSerialNumber() const noexcept { return Index >> NUM_ENT_ENTRY_BITS; }
	bool valid_entity() const noexcept;

	auto operator<=>(const IBaseHandle&) const = default;
	IBaseHandle& operator=(nullptr_t) { Index = INVALID_EHANDLE_INDEX; return *this; }

	operator bool() const noexcept
	{
		return Index != INVALID_EHANDLE_INDEX;
	}

private:
	// The low NUM_SERIAL_BITS hold the index. If this value is less than MAX_EDICTS, then the entity is networkable.
	// The high NUM_SERIAL_NUM_BITS bits are the serial number.
	unsigned long	Index{ INVALID_EHANDLE_INDEX };
};

constexpr IBaseHandle INVALID_ENTITY_HANDLE{};