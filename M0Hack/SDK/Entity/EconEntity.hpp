#pragma once

#include "BaseEntity.hpp"

class EconItemAttribute
{
public:
	uint32_t*			VTable;
	uint16_t			AttributeDefinitionIndex;
	float				FloatValue;
	unsigned int		RefundableCurrency;

public:
	EconItemAttribute() = default;
	EconItemAttribute(uint16_t index, float value) noexcept
	{
		this->AttributeDefinitionIndex = index;
		this->FloatValue = value;
	}
};

class IAttributeList
{
public:
	uint32_t*							VTable;
	ValveUtlVector<EconItemAttribute>	Attributes;

	void	SetAttribute(int index, float value) noexcept;
	float	GetAttribute(int index) noexcept;
	void	RemoveAttribute(int index) noexcept;
	int		GetNumAttributes() const noexcept { return Attributes.Count(); }
};

enum class UnusualWeaponEffect
{
	Hot = 701,
	Isotope,
	Cool,
	EnergyOrb
};

enum class TFSheens
{
	Team = 1,
	Deadly_Daffodil,
	Manndarin,
	Mean_Green,
	Agnoizing_Emerald,
	Villainous_Violet,
	Hot_Rod
};


class IEconEntityInternal : public IBaseEntityInternal
{
public:
	GAMEPROP_DECL_RECV(int,					"CEconEntity", 			"m_iItemDefinitionIndex",	ItemDefinitionIndex);
	GAMEPROP_DECL_RECV(IAttributeList,		"CEconEntity", 			"m_AttributeList",			AttributeList);
};


class IEconItemSchema
{
public:
	static IEconItemSchema* Get();
};


class _IEconLootListDefinition
{
public:
	class _Iterator
	{
		virtual ~_Iterator() = default;
		virtual void OnIterate(uint16_t) abstract;
	};

	virtual ~_IEconLootListDefinition() = default;
	[[nodiscard]] virtual bool BPublicListContents() abstract;
	[[nodiscard]] virtual const char* GetLootListHeaderLocalizationKey() abstract;
	[[nodiscard]] virtual const char* GetLootListFooterLocalizationKey() abstract;
	[[nodiscard]] virtual const char* GetLootListCollectionReference() abstract;
	virtual void EnumerateUserFacingPotentialDrops(_Iterator*) abstract;
};

class IEconLootListDefinition : public _IEconLootListDefinition
{
public:
	struct DropPeriod
	{
		uint32_t	Start;
		uint32_t	End;
	};

	struct DropItem
	{
		int				ItemOrLootlistDef;
		float			Weight;
		DropPeriod		DropTime;
	};

	struct ExtraDropItem
	{
		float			Chance;
		bool			PremiumOnly;
		const char*		LootListDefName;
		int				RequiredHolidayIndex;
		DropPeriod		DropPeriod;
	};
};