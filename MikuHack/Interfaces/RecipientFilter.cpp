
#include "RecipientFilter.h"

#include "../Helpers/Commons.h"


bool CRecipientFilter::IsReliable() const noexcept
{
	return is_reliable;
}


int CRecipientFilter::GetRecipientCount() const noexcept
{
	return recipients.size();
}


int	CRecipientFilter::GetRecipientIndex(int slot) const
{
	if (slot < 0 || slot >= GetRecipientCount())
		return -1;

	return recipients[slot];
}

void CRecipientFilter::AddAllPlayers()
{
	if (!ILocalPtr())
		return;

	recipients.clear();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		IClientShared* pEnt = GetIClientEntity(i);
		if (!::BadEntity(pEnt))
			AddRecipient(pEnt);
	}
}


void CRecipientFilter::AddRecipient(IClientShared* player)
{
	if (int index = player->entindex();
		std::find(recipients.begin(), recipients.end(), index) == recipients.end())
		recipients.push_back(index);
}

void CRecipientFilter::RemoveRecipient(IClientShared* player)
{
	if (!player)
		return;

	int index = player->entindex();
	auto pos = std::find(recipients.begin(), recipients.end(), index);
	if(pos != recipients.end())
		recipients.erase(pos);
}


void CLocalFilter::AddRecipient()
{
	recipients.clear();
	recipients.push_back(engineclient->GetLocalPlayer());
}
