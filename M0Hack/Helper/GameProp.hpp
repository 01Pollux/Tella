#pragma once

#include "NetVars_Helper.hpp"
#include "DataMap_Helper.hpp"

#include "Helper/AutoList.hpp"

#include <memory>

class IGamePropHelper
{
public:
	bool FindRecvProp(const char* class_name, const char* prop_name, CachedRecvInfo* info);
	bool FindRecvProp(const ClientClass* pCls, const char* prop_name, CachedRecvInfo* info);

	bool FindDataMap(const EntityDataMap* map, const char* name, CachedDataMapInfo* info);

private:
	ClientClass* FindClientClass(const char* class_name) const noexcept;

	bool FindInRecvTable(const RecvTable* pTable, const char* target_name, CachedRecvInfo* info);
	bool FindInDataMap(const EntityDataMap* map, const char* name, CachedDataMapInfo* info);
};


class IGameRecvProp : public IAutoList<IGameRecvProp>
{
public:
	IGameRecvProp(const char* class_name, const char* prop_name) noexcept : PropName(prop_name), ClassName(class_name) { };

	void LoadOffset(IGamePropHelper*);

	_NODISCARD ptrdiff_t GetOffset() const noexcept
	{
		return Offset;
	}

	_NODISCARD const RecvProp* GetProp() const noexcept
	{
		return Prop;
	}

private:
	ptrdiff_t Offset{ -1 };
	RecvProp* Prop{ };

	const char* ClassName{ };
	const char* PropName{ };
};

class IGameDataMapProp
{
public:
	IGameDataMapProp(const char* prop_name) noexcept : PropName(prop_name) { };

	void LoadOffset(const IBaseEntityInternal*);

	_NODISCARD ptrdiff_t GetOffset() const noexcept
	{
		return Offset;
	}

	_NODISCARD const typedescription_t* GetProp() const noexcept
	{
		return Prop;
	}

private:
	ptrdiff_t Offset{ -1 };
	typedescription_t* Prop{ };

	const char* PropName{ };
};



template<typename DataType, typename OffsetManager, OffsetManager* OffsetMgr, const size_t* DeltaOffset, bool IsDataMap, ptrdiff_t ExtraOffset = 0>
class IGamePropManager
{
	template<typename OtherType>
	using OtherPropManager = IGamePropManager<OtherType, OffsetManager, OffsetMgr, DeltaOffset, IsDataMap, ExtraOffset>;

public:
	using reference = DataType&;
	using const_reference = const DataType&;
	using pointer = DataType*;
	using const_pointer = const DataType*;

public:
	IGamePropManager() = delete;
	IGamePropManager(const IGamePropManager&) = delete; IGamePropManager& operator=(const IGamePropManager&) = delete;
	IGamePropManager(IGamePropManager&&) = delete; IGamePropManager& operator=(IGamePropManager&&) = delete;

	operator reference() noexcept { return GetVarRef(); }
	operator const_reference() const noexcept { return GetVarConstRef(); }

	template<typename OtherType>
	const_reference operator=(const OtherPropManager<OtherType>& o) { return this->GetVarRef() = static_cast<const_reference>(o.GetVarConstRef()); }
	template<typename OtherType>
	const_reference operator=(const OtherType& o) { return this->GetVarRef() = static_cast<const_reference>(o); }


	template<typename OtherType> const DataType& operator+=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() + static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator-=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() - static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator*=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() * static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator/=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() / static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator%=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() % static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator&=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() & static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator|=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() | static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator^=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() ^ static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator<<=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() << static_cast<const_reference>(o); }
	template<typename OtherType> const DataType& operator>>=(const OtherType& o) noexcept { return GetVarRef() = GetVarConstRef() >> static_cast<const_reference>(o); }

	template<typename OtherType = DataType> _NODISCARD auto operator<=>(const OtherType& o) const noexcept { return GetVarConstRef() <=> static_cast<const_reference>(o); }
	template<typename OtherType = DataType> _NODISCARD bool operator!=(const OtherType& o) const noexcept { return GetVarConstRef() != static_cast<const_reference>(o); }

	template<typename OtherType = DataType> _NODISCARD auto operator<=>(const OtherPropManager<OtherType>& man) const noexcept { return GetVarConstRef() <=> static_cast<const_reference>(man.GetVarConstRef()); }
	template<typename OtherType = DataType> _NODISCARD bool operator!=(const OtherPropManager<OtherType>& man) const noexcept { return GetVarConstRef() != static_cast<const_reference>(man.GetVarConstRef()); }

	auto operator+() const noexcept { return +this->GetVarConstRef(); }
	auto operator-() const noexcept { return -this->GetVarConstRef(); }
	auto operator~() const noexcept { return ~this->GetVarConstRef(); }

	template<typename OtherType> friend _NODISCARD auto operator+(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() + o; }
	template<typename OtherType> friend _NODISCARD auto operator+(const OtherType& o, const OtherPropManager<OtherType>& man) { return o + man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator-(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() - o; }
	template<typename OtherType> friend _NODISCARD auto operator-(const OtherType& o, const OtherPropManager<OtherType>& man) { return o - man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator*(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() * o; }
	template<typename OtherType> friend _NODISCARD auto operator*(const OtherType& o, const OtherPropManager<OtherType>& man) { return o * man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator/(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() / o; }
	template<typename OtherType> friend _NODISCARD auto operator/(const OtherType& o, const OtherPropManager<OtherType>& man) { return o / man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator%(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() % o; }
	template<typename OtherType> friend _NODISCARD auto operator%(const OtherType& o, const OtherPropManager<OtherType>& man) { return o % man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator&(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() & o; }
	template<typename OtherType> friend _NODISCARD auto operator&(const OtherType& o, const OtherPropManager<OtherType>& man) { return o & man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator|(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() | o; }
	template<typename OtherType> friend _NODISCARD auto operator|(const OtherType& o, const OtherPropManager<OtherType>& man) { return o | man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator^(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() ^ o; }
	template<typename OtherType> friend _NODISCARD auto operator^(const OtherType& o, const OtherPropManager<OtherType>& man) { return o ^ man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator<<(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() << o; }
	template<typename OtherType> friend _NODISCARD auto operator<<(const OtherType& o, const OtherPropManager<OtherType>& man) { return o << man.GetVarConstRef(); }
	template<typename OtherType> friend _NODISCARD auto operator>>(const OtherPropManager<OtherType>& man, const OtherType& o) { return man.GetVarConstRef() >> o; }
	template<typename OtherType> friend _NODISCARD auto operator>>(const OtherType& o, const OtherPropManager<OtherType>& man) { return o >> man.GetVarConstRef(); }

	_NODISCARD const_pointer operator&() const noexcept		{ return this->GetVarConstPtr(); }
	_NODISCARD pointer operator&() noexcept					{ return this->GetVarPtr(); }

	_NODISCARD const_pointer operator->() const noexcept		{ return this->GetVarConstPtr(); }
	_NODISCARD pointer operator->() noexcept					{ return this->GetVarPtr(); }

	_NODISCARD const auto& operator[](size_t i) const noexcept { return GetVarConstRef()[i]; }
	_NODISCARD auto& operator[](size_t i) noexcept				{ return GetVarRef()[i]; }

	_NODISCARD reference operator++()							{ return GetVarRef() = GetVarConstRef() + 1; }
	_NODISCARD reference operator--()							{ return GetVarRef() = GetVarConstRef() - 1; }

	template<typename OtherType = size_t> _NODISCARD auto operator[](const OtherType& idx) const noexcept { return this->GetVarConstRef()[idx]; }
	template<typename OtherType = size_t> _NODISCARD auto operator[](const OtherType& idx) noexcept { return this->GetVarRef()[idx]; }

	_NODISCARD auto begin() const noexcept				{ return this->GetVarConstRef().begin(); }
	_NODISCARD auto begin() noexcept					{ return this->GetVarRef().begin(); }

	_NODISCARD auto end() const noexcept				{ return this->GetVarConstRef().end(); }
	_NODISCARD auto end() noexcept						{ return this->GetVarRef().end(); }

	void set(const_reference o) noexcept			{ GetVarRef() = o; }

	_NODISCARD reference get() noexcept				{ return GetVarRef(); }
	_NODISCARD const_reference get() const noexcept	{ return GetVarConstRef(); }

	_NODISCARD pointer data() noexcept					{ return GetVarPtr(); }
	_NODISCARD const_pointer data() const noexcept		{ return GetVarConstPtr(); }

	_NODISCARD const RecvProp* prop() const noexcept	{ return OffsetMgr->GetProp(); }
	_NODISCARD const ptrdiff_t offset() const noexcept	{ return GetOffset(); }

	template<typename = std::enable_if_t<std::is_array_v<DataType>>>
	_NODISCARD static constexpr size_t size() noexcept { return sizeof(DataType) / sizeof(std::remove_extent_t<DataType>); }


private:
	uintptr_t GetThisPtr() noexcept
	{
		return reinterpret_cast<uintptr_t>(this) - *DeltaOffset;
	}
	
	const uintptr_t GetThisPtr() const noexcept
	{
		return reinterpret_cast<const uintptr_t>(this) - *DeltaOffset;
	}

	ptrdiff_t GetOffset() const noexcept
	{
		if constexpr (!OffsetMgr)
			return ExtraOffset;
		else if constexpr (IsDataMap)
			if (!prop())
				OffsetMgr->LoadOffset(static_cast<const IBaseEntityInternal*>(GetThisPtr()));
		
		return OffsetMgr->GetOffset() + ExtraOffset;
	}

	pointer GetVarPtr() noexcept
	{
		return reinterpret_cast<pointer>(GetThisPtr() + GetOffset());
	}

	const_pointer GetVarConstPtr() const noexcept
	{
		return reinterpret_cast<const_pointer>(GetThisPtr() + GetOffset());
	}

	reference GetVarRef() noexcept
	{
		return *GetVarPtr();
	}

	const_reference GetVarConstRef() const noexcept
	{
		return *GetVarConstPtr();
	}
};


#define GAMEPROP_DECL_PROP(TYPE, MGR, CUSTOM_NAME, IS_DATAMAP, ...) \
	static size_t _##CUSTOM_NAME##_mgr_offs; \
	using _##CUSTOM_NAME##_mgr_t = IGamePropManager<TYPE, std::remove_pointer_t<decltype(MGR)>, MGR, &_##CUSTOM_NAME##_mgr_offs, IS_DATAMAP, __VA_ARGS__>; \
	_##CUSTOM_NAME##_mgr_t CUSTOM_NAME

#define GAMEPROP_IMPL_OFFSET(CLASS, NAME) size_t CLASS::_##NAME##_mgr_offs = offsetof(CLASS, NAME)


#define GAMEPROP_DECL_RECV(TYPE, CLASSNAME, PROPNAME, CUSTOM_NAME, ...) \
	static inline IGameRecvProp _##CUSTOM_NAME##_rcv{ CLASSNAME, PROPNAME }; \
	GAMEPROP_DECL_PROP(TYPE, &_##CUSTOM_NAME##_rcv, CUSTOM_NAME, false, __VA_ARGS__)

#define GAMEPROP_IMPL_RECV(CLASS, NAME) GAMEPROP_IMPL_OFFSET(CLASS, NAME)


#define GAMEPROP_DECL_DTM(TYPE, PROPNAME, CUSTOM_NAME, ...) \
	static inline IGameDataMapProp _##CUSTOM_NAME##_dtm{ PROPNAME }; \
	GAMEPROP_DECL_PROP(TYPE, &_##CUSTOM_NAME##_dtm, CUSTOM_NAME, false, __VA_ARGS__)

#define GAMEPROP_IMPL_DTM(CLASS, NAME) GAMEPROP_IMPL_OFFSET(CLASS, NAME)


#define GAMEPROP_DECL_OFFSET(TYPE, CUSTOM_NAME, OFFSET) \
	GAMEPROP_DECL_PROP(TYPE, nullptr, CUSTOM_NAME, true, OFFSET)