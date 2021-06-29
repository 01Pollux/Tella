#include "EconEntity.hpp"
#include "Library/Lib.hpp"

GAMEPROP_IMPL_RECV(IEconEntityInternal, ItemDefinitionIndex);
GAMEPROP_IMPL_RECV(IEconEntityInternal, AttributeList);

void IAttributeList::SetAttribute(int index, float value) noexcept
{
	for (int i = 0; i < Attributes.Count(); i++)
	{
		EconItemAttribute& attr = Attributes[i];
		if (attr.AttributeDefinitionIndex == index)
		{
			attr.FloatValue = value;
			return;
		}
	}

	if (Attributes.Count() >= 15)
		return;

	Attributes.AddToTail(EconItemAttribute(index, value));
}

float IAttributeList::GetAttribute(int index) noexcept
{
	for (int i = 0; i < Attributes.Count(); i++)
		if (Attributes[i].AttributeDefinitionIndex == index)
			return Attributes[i].FloatValue;

	return 0.0f;
}

void IAttributeList::RemoveAttribute(int index) noexcept
{
	for (int i = 0; i < Attributes.Count(); i++)
	{
		if (Attributes[i].AttributeDefinitionIndex == index)
		{
			Attributes.Remove(i); 
			break;
		}
	}
}


IEconItemSchema* IEconItemSchema::Get()
{
	static IFuncThunk<IEconItemSchema*> GetItemSchema(M0CLIENT_DLL, "GetItemSchema");
	return GetItemSchema();
}

