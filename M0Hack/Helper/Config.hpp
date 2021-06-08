#pragma once

#include <array>
#include <map>
#include <type_traits>

#include "Json.hpp"

#define M0CONFIG_VALUE_KEY			"value"
#define M0CONFIG_DESCRIPTION_KEY	"description"

#define M0CONFIG_NULL_DESCRIPTION	nullptr

#define M0CONFIG_NULL_CONFIG		nullptr

#define M0CONFIG_CHEAT_PATH			"./Miku/Cheat.json"

#define M0CONFIG_BEGIN namespace M0Config {

#define M0CONFIG M0Config:: 

#define M0CONFIG_END }

#define M0CONFIG_INHERIT_FROM(CLASSNAME, DATATYPE, ...) \
public: \
	CLASSNAME(const std::string& key, const DATATYPE& def, const char* des) noexcept : __VA_ARGS__(key, def, des) { } \
	CLASSNAME(const DATATYPE& def, const char* des) noexcept : __VA_ARGS__(def, des) { } \
private:


#define M0CONFIG_IMPL_MAP(DATATYPE) \
protected: \
void _read(const Json::Value& inc) noexcept final \
{ \
	auto map = get_namemap(); \
	this->get() = map[inc.asString()]; \
} \
 \
void _write(Json::Value& out) const noexcept final \
{ \
	for (auto map = get_namemap(); \
		auto it : map) \
	{ \
		if (it.second == this->get()) \
		{ \
			out = it.first; \
			break; \
		} \
	} \
} \
 \
private: \
	static std::map<std::string, DATATYPE> get_namemap() \



M0CONFIG_BEGIN;

class M0VarStorage
{
public:
	/// <summary>	Export a config variable (compile time)	</summary>
	/// 
	/// <param name="strs">	Key Name	</param>
	/// <param name="des">	Description (optional)	</param>
	M0VarStorage(const std::string& strs, const char* des = M0CONFIG_NULL_DESCRIPTION) noexcept;
	M0VarStorage(const char* des = M0CONFIG_NULL_DESCRIPTION) noexcept { Description = des; };

	virtual ~M0VarStorage() noexcept;
	M0VarStorage(const M0VarStorage&) = delete;		M0VarStorage& operator=(const M0VarStorage&) = delete;
	M0VarStorage(M0VarStorage&&) = delete;			M0VarStorage& operator=(M0VarStorage&&) = delete;

	void set_description(const char* des)		noexcept { Description = des; }
	_NODISCARD const char* get_description()	const noexcept { return Description; }
	_NODISCARD bool has_description()			const noexcept { return Description != M0CONFIG_NULL_DESCRIPTION; }

public:
	/// <summary>	Internal only	</summary>
	static void write_var();

	/// <summary>	Internal only	</summary>
	static void read_var();

	/// <summary>	Export a config variable	</summary>
	/// 
	/// <param name="key">	Key Name	</param>
	/// <param name="inst">	Pointer to config variable	</param>
	/// <param name="des">	Description (optional)	</param>
	/// 
	/// <returns> true if variable successfully inserted, false otherwise / inserted @compile time 	</returns>
	static bool export_var(const std::string& key, M0VarStorage* inst, const char* des = M0CONFIG_NULL_DESCRIPTION);

	/// <summary>	Export a config variable	</summary>
	/// 
	/// <param name="key">	Key Name	</param>
	/// 
	/// <returns> true if variable successfully inserted, false otherwise / inserted @compile time 	</returns>
	bool export_var(const std::string& key)
	{
		return export_var(key, this, Description);
	}

	_NODISCARD static M0VarStorage* import_var(const std::string& key);

	template<class M0VarType, typename = std::enable_if_t<std::is_base_of_v<M0VarStorage, M0VarType>>>
	_NODISCARD static void import_var(M0VarType** var, const std::string& key)
	{
		*var = static_cast<M0VarType*>(import_var(key));
	}

	/// <summary> Remove a config variable </summary>
	/// 
	/// <param name="inst"> Pointer to config variable </param>
	static void detach_var(M0VarStorage* inst);

	/// <summary> Remove a config variable </summary>
	/// 
	/// <param name="str"> config variable name </param>
	static void detach_var(const std::string& str);

	static void print_vars();

protected:
	virtual void _write(Json::Value&) const abstract;
	virtual void _read(const Json::Value&) abstract;

private:
	const char* Description;
	static inline std::map<std::string, M0VarStorage*> VarStorage;
};


template<typename _Ty>
class M0Var_Internal : public M0VarStorage
{
	template<class _oT>					struct _is_array : std::is_array<_oT> {};
	template<class _oT, size_t Size>	struct _is_array<std::array<_oT, Size>> : std::true_type {};
public:
	using type = _Ty;
	using reference = type&;
	using const_reference = const type&;
	using pointer = type*;
	using const_pointer = const type*;

	static constexpr bool is_basic_type = std::disjunction_v<std::is_same<type, int>, std::is_same<type, bool>, std::is_same<type, float>, std::is_same<type, std::string>>;
	static constexpr bool is_array_type = _is_array<type>::value;

	M0Var_Internal(const std::string& strs, const _Ty& def, const char* des = M0CONFIG_NULL_DESCRIPTION) noexcept : M0VarStorage(strs, des), Var(def) { }
	M0Var_Internal(const _Ty& def, const char* des = M0CONFIG_NULL_DESCRIPTION) noexcept : M0VarStorage(des), Var(def) { }

	_NODISCARD reference get()				noexcept			{ return Var; }
	_NODISCARD const_reference get()		const noexcept		{ return Var; }

	operator reference()					noexcept			{ return Var; }
	operator const_reference()				const noexcept		{ return Var; }

	_NODISCARD auto data()					noexcept			{ if constexpr (is_array_type) return Var.data(); else return &Var; }
	_NODISCARD const auto data()			const noexcept		{ if constexpr (is_array_type) return Var.data(); else return &Var; }

	_NODISCARD auto operator[](size_t idx)	noexcept			{ return Var[idx]; }
	const auto operator[](size_t idx)		const noexcept		{ return Var[idx]; }

	_NODISCARD size_t size()				const noexcept		{ return Var.size(); }

	auto& operator=(const_reference o)							{ Var = o; return *this; }

protected:
	void _write(Json::Value& out) const override
	{
		if constexpr (is_basic_type)
			out = Var;
	}

	void _read(const Json::Value& inc) override
	{
		if constexpr (std::is_same_v<type, bool>)
			Var = inc.asBool();
		else if constexpr (std::is_same_v<type, int>)
			Var = inc.asInt();
		else if constexpr (std::is_same_v<type, float>)
			Var = inc.asFloat();
		else if constexpr (std::is_same_v<type, std::string>)
			Var = inc.asString();
	}

private:
	_Ty Var;
};


template<typename _Ty, bool _HasNotifier>
class M0Var_Wrapper : public M0Var_Internal<_Ty>
{
	M0CONFIG_INHERIT_FROM(M0Var_Wrapper, _Ty, M0Var_Internal<_Ty>);

public:
	using notifier_type = void(*)(const M0Var_Internal<_Ty>*, const _Ty&);
	using M0Var_Internal<_Ty>::get;

	void notify_and_change(const _Ty& new_val)
	{
		if (get() != new_val)
		{
			notify();
			get() = new_val;
		}
	}

	void notify()
	{
		for (const auto& callback : Callbacks)
			callback(this, get());
	}

private:
	std::vector<notifier_type> Callbacks;
};


template<typename _Ty>
class M0Var_Wrapper<_Ty, false> : public M0Var_Internal<_Ty>
{
	M0CONFIG_INHERIT_FROM(M0Var_Wrapper, _Ty, M0Var_Internal<_Ty>);

public:
	using M0Var_Internal<_Ty>::operator=;
};


template<class M0VarType>
class M0VarRef_Wrapper
{
public:
	using type = M0VarType;
	using true_type = M0VarType::type;
	using reference = true_type&;
	using const_reference = const true_type&;
	using pointer = true_type*;
	using const_pointer = const true_type*;

	M0VarRef_Wrapper() = default;
	M0VarRef_Wrapper(const std::string& key) noexcept { init(key); }

	bool init(const std::string& key) noexcept
	{
		M0VarType::import_var(&VarPointer, key);
		return VarPointer != nullptr;
	}

	operator M0VarStorage&()				noexcept		{ return *VarPointer; }
	operator const M0VarStorage&()			const noexcept	{ return *VarPointer; }

	operator reference()					noexcept		{ return VarPointer->get();	}
	operator const_reference()				const noexcept	{ return VarPointer->get(); }

	_NODISCARD reference get()				noexcept		{ return VarPointer->get();	}
	_NODISCARD const_reference get()		const noexcept	{ return VarPointer->get(); }
	
	_NODISCARD auto& operator[](size_t i)	noexcept		{ return VarPointer->get()[i]; }
	const auto& operator[](size_t i)		const noexcept	{ return VarPointer->get()[i]; }

	_NODISCARD auto data()					noexcept		{ return VarPointer->data(); }
	_NODISCARD auto data()					const noexcept	{ return VarPointer->data(); }
	
	_NODISCARD size_t size()				const noexcept	{ return VarPointer->size(); }

	auto& operator=(const_reference o)						{ return VarPointer->operator=(o); }

private:
	M0VarType* VarPointer{ };
};


template<typename _Ty, bool _HasNotifier>				using Custom		= M0Var_Wrapper<_Ty, _HasNotifier>;
template<typename _Ty, size_t _Size, bool _HasNotifier>	using CustomArray	= M0Var_Wrapper<std::array<_Ty, _Size>, _HasNotifier>;
template<typename _Ty, bool _HasNotifier>				using CustomRef		= M0VarRef_Wrapper<Custom<_Ty, _HasNotifier>>;
template<typename _Ty, size_t _Size, bool _HasNotifier>	using CustomArrayRef= M0VarRef_Wrapper<CustomArray<_Ty, _Size, _HasNotifier>>;

using Bool = Custom<bool, false>;
using BoolN = Custom<bool, true>;
using BoolRef = M0VarRef_Wrapper<Bool>;
using BoolRefN = M0VarRef_Wrapper<BoolN>;

using Int = Custom<int, false>;
using IntN = Custom<int, true>;
using IntRef = M0VarRef_Wrapper<Int>;
using IntRefN = M0VarRef_Wrapper<IntN>;

using Float = Custom<float, false>;
using FloatN = Custom<float, true>;
using FloatRef = M0VarRef_Wrapper<Float>;
using FloatRefN = M0VarRef_Wrapper<FloatN>;

using String = Custom<std::string, false>;
using StringN = Custom<std::string, true>;
using StringRef = M0VarRef_Wrapper<String>;
using StringRefN = M0VarRef_Wrapper<StringN>;

template<typename _Ty, size_t _Size> using Array		= CustomArray<_Ty, _Size, false>;
template<typename _Ty, size_t _Size> using ArrayN		= CustomArray<_Ty, _Size, true>;
template<typename _Ty, size_t _Size> using ArrayRef		= CustomArrayRef<_Ty, _Size, false>;
template<typename _Ty, size_t _Size> using ArrayRefN	= CustomArrayRef<_Ty, _Size, true>;



// Json : [0] = "x", [1] = "y"
template<typename _Ty, bool _HasNotifier>
class ArrayXY_Wrapper final : public CustomArray<_Ty, 2, _HasNotifier>
{
public:
	using base_type = CustomArray<_Ty, 2, _HasNotifier>;
	M0CONFIG_INHERIT_FROM(ArrayXY_Wrapper, typename base_type::type, base_type);

public:
	using base_type::get;

	_NODISCARD _Ty& x()	noexcept		{ return get()[0]; }
	_NODISCARD _Ty x()		const noexcept	{ return get()[0]; }
	_NODISCARD _Ty& y()	noexcept		{ return get()[1]; }
	_NODISCARD _Ty y()		const noexcept	{ return get()[1]; }
	
	void _write(Json::Value& out) const final
	{
		out["x"] = x();
		out["y"] = y();
	}

	void _read(const Json::Value& inc) final
	{
		if constexpr (std::is_same_v<_Ty, float>)
		{
			if (!inc["x"].isNull())
				x() = inc["x"].asFloat();
			if (!inc["y"].isNull())
				y() = inc["y"].asFloat();
		}
		else
		{
			if (!inc["x"].isNull())
				x() = static_cast<_Ty>(inc["x"].asInt());
			if (!inc["y"].isNull())
				y() = static_cast<_Ty>(inc["y"].asInt());
		}
	}
};

template<typename _Ty>	using ArrayXY		= Array<_Ty, 2>;
template<typename _Ty>	using ArrayXYN		= ArrayN<_Ty, 2>;
template<typename _Ty>	using ArrayXYRef	= ArrayRef<_Ty, 2>;
template<typename _Ty>	using ArrayXYRefN	= ArrayRefN<_Ty, 2>;

M0CONFIG_END;