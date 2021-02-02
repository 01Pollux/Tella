
#include "sdk.h"
#include "../Interfaces/CBaseEntity.h"

class CUserCmd;

namespace Globals
{
	CUserCmd*		m_pUserCmd		= nullptr;
	bool*			bSendPacket		= nullptr;
	bool			m_bMenuActive	= false;
}

int (WINAPIV* __vsnprintf)(char*, size_t, const char*, va_list) = _vsnprintf;
int (WINAPIV* __vsnwprintf)(wchar_t*, size_t, const wchar_t*, va_list) = _vsnwprintf;


void IClientEntityListener::AddEntityListener()
{
	auto listener = FindEntityListener();
	if(listener->Find(this) == listener->InvalidIndex())
		listener->AddToTail(this);
}

void IClientEntityListener::RemoveEntityListener()
{
	auto listener = FindEntityListener();
	listener->FindAndRemove(this);
}
