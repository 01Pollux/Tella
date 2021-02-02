
#include "BytePatch.h"


void IBytePatcher::Patch()
{
	if (!m_bPatched)
	{
		void* start = reinterpret_cast<void*>(this->m_pBase);

		DWORD tmp;
		if (VirtualProtect(start, m_isize, PAGE_EXECUTE_READWRITE, &tmp))
		{
			memcpy(start, &m_newBytes[0], m_isize);
			VirtualProtect(start, m_isize, tmp, &tmp);
			m_bPatched = true;
		}
	}
}

void IBytePatcher::Restore()
{
	if (m_bPatched)
	{
		void* start = reinterpret_cast<void*>(this->m_pBase);

		DWORD tmp;
		if (VirtualProtect(start, this->m_isize, PAGE_EXECUTE_READWRITE, &tmp))
		{
			memcpy(start, &m_oldBytes[0], this->m_isize);
			VirtualProtect(start, this->m_isize, tmp, &tmp);
			m_bPatched = false;
		}
	}
}
