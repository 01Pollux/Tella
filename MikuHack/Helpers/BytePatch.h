#pragma once

#include <Windows.h>
#include <vector>
#include <memory>

class IBytePatcher
{
public:
	using ByteVec = std::vector<uint8_t>;
	IBytePatcher(uintptr_t pBase, uint16_t offset, ByteVec patch): m_newBytes{ patch }
	{
		m_isize = patch.size();
		this->m_pBase = pBase + offset;
		m_oldBytes.resize(m_isize);
		m_newBytes.resize(m_isize);

		memcpy(&m_oldBytes[0], reinterpret_cast<void*>(this->m_pBase), m_isize);
	}
	~IBytePatcher()
	{
		this->Restore();
	}
	void Patch();
	void Restore();

private:
	ByteVec m_oldBytes;
	ByteVec m_newBytes;
	size_t m_isize;
	uintptr_t m_pBase = NULL;
	bool m_bPatched = false;
};
