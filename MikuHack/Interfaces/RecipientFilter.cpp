
#include "RecipientFilter.h"
#include "../Interfaces/CBaseEntity.h"

CRecipientFilter::CRecipientFilter()
{
	m_bReliable = false;
	m_Recipients.clear();
}

CRecipientFilter::~CRecipientFilter()
{

}

bool CRecipientFilter::IsReliable() const
{
	return m_bReliable;
}


int CRecipientFilter::GetRecipientCount() const
{
	return m_Recipients.size();
}


int	CRecipientFilter::GetRecipientIndex(int slot) const
{
	if (slot < 0 || slot >= GetRecipientCount())
		return -1;

	return m_Recipients[slot];
}

void CRecipientFilter::AddAllPlayers()
{
	if (!pLocalPlayer)
		return;

	m_Recipients.clear();

	IClientEntity* pEnt;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		pEnt = GetClientEntityW(i);
		if (pEnt && !pEnt->IsDormant())
			AddRecipient(pEnt);
	}
}


void CRecipientFilter::AddRecipient(IClientEntity* player)
{
	int index = player->entindex();

	if (std::find(m_Recipients.begin(), m_Recipients.end(), index) != m_Recipients.end())
		return;

	m_Recipients.push_back(index);
}

void CRecipientFilter::RemoveRecipient(IClientEntity* player)
{
	if (!player)
		return;

	int index = player->entindex();
	auto pos = std::find(m_Recipients.begin(), m_Recipients.end(), index);
	if(pos != m_Recipients.end())
		m_Recipients.erase(pos);
}


void CLocalFilter::AddRecipient()
{
	m_Recipients.clear();
	m_Recipients.push_back(engineclient->GetLocalPlayer());
}
