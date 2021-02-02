#pragma once

#include <irecipientfilter.h>
#include <vector>

class IClientEntity;
class CRecipientFilter : public IRecipientFilter
{
public:
	CRecipientFilter();
	virtual			~CRecipientFilter();

	virtual bool	IsReliable(void) const;

	virtual int		GetRecipientCount(void) const;
	virtual int		GetRecipientIndex(int slot) const;

	virtual bool	IsInitMessage(void) const { return false; };

public:
	void			AddAllPlayers(void);
	void			AddRecipient(IClientEntity* player);
	void			RemoveRecipient(IClientEntity* player);

	bool				m_bReliable;
	bool				m_bInitMessage;
	std::vector<int>	m_Recipients;
};

class CLocalFilter : public CRecipientFilter
{
public:
	void AddRecipient();
};