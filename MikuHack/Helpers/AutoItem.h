#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include <Color.h>
#include <any>

#include "../Helpers/json.h"

enum VarType_t
{
	VAR_BOOL,
	VAR_INT,
	VAR_FLOAT,
	VAR_STRING
};

class AutoVarStorage
{
public:
	using _VarKey = std::string;
	using _VarHashMap = std::unordered_map<_VarKey, AutoVarStorage*>;

	static inline _VarHashMap VarList;

	AutoVarStorage(_VarKey name)
	{
		if (VarList.find(name) == VarList.end())
			VarList.insert(std::make_pair(name, this));
		else Msg("Key: %s is not unique\n", name.c_str());
	}

	static AutoVarStorage* FindInMap(_VarKey name)
	{
		return VarList[name];
	}
};


template<typename _DataType>
class AutoVar : protected AutoVarStorage
{
	using _DataPtr = std::shared_ptr<_DataType>;

public:
	AutoVar<_DataType>(_VarKey name) : AutoVarStorage(name)
	{
		Init(name, { });
	};

	AutoVar<_DataType>(_VarKey name, _DataType default_value) : AutoVarStorage(name)
	{
		Init(name, default_value);
	};

	AutoVar<_DataType> operator=(const AutoVar<_DataType>& other) = delete;

	AutoVar<_DataType>(const AutoVar<_DataType>&) = delete;

	operator AutoVar<_DataType>() = delete;

	operator _DataType() const noexcept
	{
		return *var;
	}

	constexpr _DataType& operator*() const noexcept
	{
		return *var;
	}

	constexpr _DataType operator->() const noexcept
	{
		return *var;
	}

	constexpr _DataType operator=(const _DataType& other) const noexcept
	{
		*var = other;
		return *var;
	}

	constexpr _DataType* get() const noexcept
	{
		return var.get();
	}

private:
	void Init(_VarKey name, const _DataType& default_value)
	{
		var = reinterpret_cast<AutoVar<_DataType>*>(FindInMap(name))->var;
		if (!var)
			var = std::make_shared<_DataType>(default_value);
	}

	_DataPtr var;
};

template<typename _DataType, size_t _DataSize>
class AutoArray : protected AutoVarStorage
{
	using _DataArr = std::array<_DataType, _DataSize>;
	using _DataPtr = std::shared_ptr<_DataArr>;

public:
	AutoArray<_DataType, _DataSize>(_VarKey name) : AutoVarStorage(name)
	{
		Init(name, { });
	};

	AutoArray<_DataType, _DataSize>(_VarKey name, const _DataType& default_value) : AutoVarStorage(name)
	{
		Init(name, default_value);
	}

	AutoArray<_DataType, _DataSize>(const AutoArray<_DataType, _DataSize>&) = delete;

	_DataType* data() noexcept
	{
		return var->data();
	}

	_DataArr& get() noexcept
	{
		return *var;
	}

	_DataType& operator[](size_t pos)
	{
		return var->at(pos);
	}

	void Set(const _DataArr& other)
	{
		*var = other;
	}

	constexpr size_t size() const noexcept
	{
		return _DataSize;
	}

private:
	void Init(_VarKey name, const _DataType& default_value)
	{
		var = reinterpret_cast<AutoArray<_DataType, _DataSize>*>(FindInMap(name))->var;
		if (!var)
		{
			var = std::make_shared<_DataArr>();
			var->fill(default_value);
		}
	}

	_DataPtr var;
};

using AutoChar = AutoVar<char8_t>;
using AutoBool = AutoVar<bool>;
using AutoInt = AutoVar<int>;
using AutoPtr = AutoVar<void*>;
using AutoFloat = AutoVar<float>;
using AutoString = AutoVar<std::string>;
using AutoColor = AutoArray<char8_t, 4>;

