#pragma once

#include <stdint.h>
#include <memory>


class PatchBuffer
{
public:
	uint8_t& operator[](size_t offset) noexcept { return getbyte(offset); }
	const uint8_t operator[](size_t offset) const noexcept { return getbyte(offset); }

	uint8_t& getbyte(size_t offset) noexcept;
	uint8_t getbyte(size_t offset) const noexcept;
	void setbyte(size_t offset, const uint8_t) noexcept;

	uint16_t getword(size_t offset) const noexcept;
	void setword(size_t offset, const uint16_t) noexcept;

	uint32_t getdword(size_t offset) const noexcept;
	void setdword(size_t offset, const uint32_t) noexcept;

	uint64_t getqword(size_t offset) const noexcept;
	void setqword(size_t offset, const uint64_t) noexcept;

	float getfloat(size_t offset) const noexcept;
	void setfloat(size_t offset, const float) noexcept;
	
	void get(size_t offset, uint8_t*, size_t) const noexcept;
	void set(size_t offset, const uint8_t*, size_t) noexcept;
	void set(size_t offset, uint8_t, size_t) noexcept;
	void set(uint8_t bytes) noexcept { set(0, bytes, Size); }

	const uint8_t* data() const noexcept { return Buffer.get(); }
	uint8_t* data() noexcept { return Buffer.get(); }

	size_t size() const noexcept { return Size; }

	explicit PatchBuffer(size_t size) noexcept : Size(size) { Buffer = std::make_unique<uint8_t[]>(size); }
	explicit PatchBuffer(const uint8_t* other, size_t size) noexcept : Size(size) 
	{
		Buffer = std::make_unique<uint8_t[]>(size);
		set(0, other, size);
	}
	PatchBuffer(const PatchBuffer&) = delete;	PatchBuffer& operator=(const PatchBuffer&) = delete;
	PatchBuffer(PatchBuffer&&) = default;		PatchBuffer& operator=(PatchBuffer&&) = default;
	~PatchBuffer() = default;

private:
	std::unique_ptr<uint8_t[]> Buffer{ };
	size_t Size;
};


class IPatch
{
public:
	explicit IPatch(void* adr, size_t size) noexcept : Address(static_cast<uint8_t*>(adr)), RestoreBuffer(static_cast<const uint8_t*>(adr), size) { }

	void patch(const PatchBuffer& patch) noexcept;
	void restore() noexcept;

	bool is_patched() const noexcept { return IsPatched; }

private:
	uint8_t* Address{ };
	PatchBuffer RestoreBuffer;
	bool IsPatched = false;
};


void IPatch::patch(const PatchBuffer& patch) noexcept
{
	if (IsPatched)
		return;
	IsPatched = true;

	for (size_t i = 0; i < patch.size(); i++)
	{
		Address[i] = patch[i];
	}
}

void IPatch::restore() noexcept
{
	if (!IsPatched)
		return;
	IsPatched = false;

	for (size_t i = 0; i < RestoreBuffer.size(); i++)
	{
		Address[i] = RestoreBuffer[i];
	}
}


inline uint8_t PatchBuffer::getbyte(size_t offset) const noexcept
{
	return Buffer[offset];
}

inline uint8_t& PatchBuffer::getbyte(size_t offset) noexcept
{
	return Buffer[offset];
}

inline void PatchBuffer::setbyte(size_t offset, const uint8_t byte) noexcept
{
	Buffer[offset] = byte;
}

inline uint16_t PatchBuffer::getword(size_t offset) const noexcept
{
	return reinterpret_cast<const uint16_t*>(data())[offset];
}

inline void PatchBuffer::setword(size_t offset, const uint16_t word) noexcept
{
	reinterpret_cast<uint16_t*>(data())[offset] = word;
}

inline uint32_t PatchBuffer::getdword(size_t offset) const noexcept
{
	return reinterpret_cast<const uint32_t*>(data())[offset];
}

inline void PatchBuffer::setdword(size_t offset, const uint32_t word) noexcept
{
	reinterpret_cast<uint32_t*>(data())[offset] = word;
}

inline uint64_t PatchBuffer::getqword(size_t offset) const noexcept
{
	return reinterpret_cast<const uint64_t*>(data())[offset];
}

inline void PatchBuffer::setqword(size_t offset, const uint64_t word) noexcept
{
	reinterpret_cast<uint64_t*>(data())[offset] = word;
}

inline float PatchBuffer::getfloat(size_t offset) const noexcept
{
	return reinterpret_cast<const float*>(data())[offset];
}

inline void PatchBuffer::setfloat(size_t offset, const float word) noexcept
{
	reinterpret_cast<float*>(data())[offset] = word;
}

inline void PatchBuffer::get(size_t offset, uint8_t* data, size_t copy_size) const noexcept
{
	std::copy_n(this->data() + offset, copy_size, data);
}

inline void PatchBuffer::set(size_t offset, const uint8_t* data, size_t copy_size) noexcept
{
	std::copy_n(data, copy_size, this->data() + offset);
}

inline void PatchBuffer::set(size_t offset, uint8_t bytes, size_t set_size) noexcept
{
	std::fill_n(data() + offset, set_size, bytes);
}