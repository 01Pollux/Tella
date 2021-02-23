#pragma once

#include <irecipientfilter.h>
#include <vector>

class IClientShared;
class CRecipientFilter : public IRecipientFilter
{
public:
	CRecipientFilter()					= default;
	~CRecipientFilter()					override = default;

	bool	IsReliable()				const noexcept override;

	int		GetRecipientCount(void)		const noexcept override;
	int		GetRecipientIndex(int slot) const override;

	bool	IsInitMessage(void)			const noexcept override { return false; };

public:
	void			AddAllPlayers(void);
	void			AddRecipient(IClientShared* player);
	void			RemoveRecipient(IClientShared* player);

	bool				is_reliable{ };
	bool				is_init_msg{ };
	std::vector<int>	recipients;

public:
	CRecipientFilter(const CRecipientFilter&)				= default;
	CRecipientFilter& operator=(const CRecipientFilter&)	= default;
	CRecipientFilter(CRecipientFilter&&)					= default;
	CRecipientFilter& operator=(CRecipientFilter&&)			= default;
};

class CLocalFilter : public CRecipientFilter
{
public:
	void AddRecipient();
};