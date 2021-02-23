#pragma once

#include <unordered_map>
#include <array>
#include <Color.h>
#include <memory>
#include <string>

template<typename _DataType>
class IAutoVarStorage
{
	using _Key = const char*;
	using _Val = _DataType;

	using _Map = std::unordered_map<_Key, _Val>;
	using _Iter = typename _Map::iterator;

public:
	IAutoVarStorage(_Key key, _Val&& default_value)
	{
		this->iter = map.insert(std::make_pair(key, default_value)).first;
	}

	_Val* DataInfo()
	{
		return &iter->second;
	}

	_Key GetName()
	{
		return iter->first;
	}

	static _Map& Map() noexcept
	{
		return map;
	}

private:

	_Iter iter;
	static inline _Map map;
};

template<typename ValType>
class IAutoVar : protected IAutoVarStorage<ValType>
{
	using IAutoStorage = IAutoVarStorage<ValType>;

public:
	IAutoVar(const char* key, ValType default_value = { }) : IAutoStorage(key, std::forward<ValType>(default_value))
	{
		val = this->DataInfo();
	}

	ValType& operator=(const ValType& other) noexcept
	{
		*val = other;
		return *val;
	}

	ValType& operator*() const noexcept
	{
		return *val;
	}

	operator ValType&() const noexcept
	{
		return *val;
	}

	ValType* const operator&() noexcept
	{
		return val;
	}

private:
	ValType* val{ };
};

 
template<typename ValType, size_t _DataSize>
class IAutoArray : protected IAutoVarStorage<std::array<ValType, _DataSize>>
{
	using ValArray = std::array<ValType, _DataSize>;
	using IAutoStorage = IAutoVarStorage<ValArray>;

public:
	IAutoArray(const char* key, ValArray default_value = { }) : IAutoStorage(key, std::forward<ValArray>(default_value))
	{
		arr = this->DataInfo();
	}

	ValArray& operator=(const ValType& other) noexcept
	{
		*arr = other;
		return *arr;
	}

	ValArray& operator*() const noexcept
	{
		return *arr;
	}

	operator ValArray&() const noexcept
	{
		return *arr;
	}

	ValType* const operator&() noexcept
	{
		return arr->data();
	}

	ValType& operator[](size_t pos)
	{
		return arr->at(pos);
	}

	constexpr size_t size() noexcept
	{
		return _DataSize;
	}
	
private:
	ValArray* arr;
};


using AutoChar = IAutoVar<char8_t>;
using AutoBool = IAutoVar<bool>;
using AutoInt = IAutoVar<int>;
using AutoFloat = IAutoVar<float>;
using AutoString = IAutoVar<std::string>;
using AutoColor = IAutoArray<char8_t, 4>;
