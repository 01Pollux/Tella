#include "CAttributes.h"
#include "../Helpers/Library.h"


void IAttributeList::SetAttribute(int iIndex, float flValue)
{
	for (int i = 0; i < m_Attributes.Count(); i++)
	{
		CEconItemAttribute& attr = m_Attributes[i];
		if (attr.m_iAttributeDefinitionIndex == iIndex)
		{
			attr.flValue = flValue;
			return;
		}
	}
	
	if (m_Attributes.Count() >= 15)
		return;

	m_Attributes.AddToTail(CEconItemAttribute(iIndex, flValue));
}

float IAttributeList::GetAttribute(int iIndex)
{
	for (int i = 0; i < m_Attributes.Count(); i++)
		if (m_Attributes[i].m_iAttributeDefinitionIndex == iIndex)
			return m_Attributes[i].flValue;

	return 0.0f;
}

void IAttributeList::RemoveAttribute(int iIndex)
{
	for (int i = 0; i < m_Attributes.Count(); i++)
	{
		if (m_Attributes[i].m_iAttributeDefinitionIndex == iIndex)
		{
			m_Attributes.Remove(i); break;
		}
	}
}