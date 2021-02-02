#pragma once

#include <tier1/utlvector.h>
#include <stdint.h>

using CEconItemAttributeDefinition = void*;
class CEconItemAttribute
{
public:
	uint32_t**			vTable;
	uint16_t			m_iAttributeDefinitionIndex;
	float				flValue;
	unsigned int		m_nRefundableCurrency;

public:
	inline CEconItemAttribute(uint16_t index, float value)
	{
		this->m_iAttributeDefinitionIndex = index;
		this->flValue = value;
	}
};

class IAttributeList
{
public:
	uint32_t** vTable;
	CUtlVector<CEconItemAttribute, CUtlMemory<CEconItemAttribute>>		m_Attributes;

public:
	void SetAttribute(int szName, float flValue);
	float GetAttribute(int iIndex);
	void RemoveAttribute(int iIndex);
	int GetNumAttributes() { return m_Attributes.Count(); }
};


enum class Unusual_Weapon
{
	HOT = 701,
	ISOTOPE,
	COOL,
	ENERGY_ORB
};

enum class Sheens
{
	TEAM_SHINE = 1,
	DEADLY_DAFFODIL,
	MANNDARIN,
	MEAN_GREEN,
	AGONIZING_EMERALD,
	VILLAINOUS_VIOLET,
	HOT_ROD
};