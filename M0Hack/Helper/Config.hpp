#pragma once

#include <array>
#include <map>
#include <type_traits>

#include "Json.hpp"

#define TCONFIG_VALUE_KEY			"value"
#define TCONFIG_DESCRIPTION_KEY		"description"

#define TCONFIG_NULL_DESCRIPTION	nullptr

#define TCONFIG_NULL_CONFIG			nullptr

#define TCONFIG_PATH				"./Miku/Cheat.json"

#define TCONFIG						tella::config:: 

#define TCONFIG_INHERIT_FROM(CLASSNAME, DATATYPE, ...) \
public: \
	CLASSNAME(const std::string& key, const DATATYPE& def, const char* des) noexcept : __VA_ARGS__(key, def, des) { } \
	CLASSNAME(const DATATYPE& def, const char* des) noexcept : __VA_ARGS__(def, des) { } \
private:


#define TCONFIG_IMPL_MAP(DATATYPE) \
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
	static std::map<std::string, DATATYPE> get_namemap()


namespace tella
{
	namespace config
	{
		class var_storage
		{
		public:
			/// <summary>	Export a config variable (compile time)	</summary>
			/// 
			/// <param name="strs">	Key Name	</param>
			/// <param name="des">	Description (optional)	</param>
			var_storage(const std::string& strs, const char* des = TCONFIG_NULL_DESCRIPTION) noexcept;
			var_storage(const char* des = TCONFIG_NULL_DESCRIPTION) noexcept { Description = des; };

			virtual ~var_storage() noexcept;
			var_storage(const var_storage&) = delete;		var_storage& operator=(const var_storage&) = delete;
			var_storage(var_storage&&) = delete;			var_storage& operator=(var_storage&&) = delete;

			void set_description(const char* des)		noexcept { Description = des; }
			_NODISCARD const char* get_description()	const noexcept { return Description; }
			_NODISCARD bool has_description()			const noexcept { return Description != TCONFIG_NULL_DESCRIPTION; }

		public:
			/// <summary>	Internal only	</summary>
			static void write_var();

			/// <summary>	Internal only	</summary>
			static void read_var();

			/// <summary> Read variable without inserting to the config storage </summary>
			static Json::Value read_var(const Json::Value& mainConfig, const std::string_view& key);

			/// <summary> Read variable without inserting to the config storage </summary>
			static Json::Value read_var(const std::string_view& key);

			/// <summary>	Export a config variable	</summary>
			/// 
			/// <param name="key">	Key Name	</param>
			/// <param name="inst">	Pointer to config variable	</param>
			/// <param name="des">	Description (optional)	</param>
			/// 
			/// <returns> true if variable successfully inserted, false otherwise / inserted @compile time 	</returns>
			static bool export_var(const std::string& key, var_storage* inst, const char* des = TCONFIG_NULL_DESCRIPTION);

			/// <summary>	Export a config variable	</summary>
			/// 
			/// <param name="key">	Key Name	</param>
			/// 
			/// <returns> true if variable successfully inserted, false otherwise / inserted @compile time 	</returns>
			bool export_var(const std::string& key)
			{
				return export_var(key, this, Description);
			}

			_NODISCARD static var_storage* import_var(const std::string& key);

			template<class M0VarType, typename = std::enable_if_t<std::is_base_of_v<var_storage, M0VarType>>>
			_NODISCARD static void import_var(M0VarType** var, const std::string& key)
			{
				*var = static_cast<M0VarType*>(import_var(key));
			}

			/// <summary> Remove a config variable </summary>
			/// 
			/// <param name="inst"> Pointer to config variable </param>
			static void detach_var(var_storage* inst);

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
			static inline std::map<std::string, var_storage*> _VarStorage;
		};


		template<typename _Ty>
		class var_internal : public var_storage
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

			var_internal(const std::string& strs, const _Ty& def, const char* des = M0CONFIG_NULL_DESCRIPTION) noexcept : var_storage(strs, des), Var(def) { }
			var_internal(const _Ty& def, const char* des = M0CONFIG_NULL_DESCRIPTION) noexcept : var_storage(des), Var(def) { }

			_NODISCARD reference get()				noexcept { return Var; }
			_NODISCARD const_reference get()		const noexcept { return Var; }

			operator reference()					noexcept { return Var; }
			operator const_reference()				const noexcept { return Var; }

			_NODISCARD auto data()					noexcept { if constexpr (is_array_type) return Var.data(); else return &Var; }
			_NODISCARD const auto data()			const noexcept { if constexpr (is_array_type) return Var.data(); else return &Var; }

			_NODISCARD auto operator[](size_t idx)	noexcept { return Var[idx]; }
			const auto operator[](size_t idx)		const noexcept { return Var[idx]; }

			_NODISCARD size_t size()				const noexcept { return Var.size(); }

			auto& operator=(const_reference o) { Var = o; return *this; }

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
		class var_wrapper : public var_internal<_Ty>
		{
			TCONFIG_INHERIT_FROM(var_wrapper, _Ty, var_internal<_Ty>);

		public:
			using notifier_type = void(*)(const M0Var_Internal<_Ty>*, const _Ty&);
			using var_internal<_Ty>::get;

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
		class var_wrapper<_Ty, false> : public var_internal<_Ty>
		{
			TCONFIG_INHERIT_FROM(var_wrapper, _Ty, var_internal<_Ty>);

		public:
			using var_internal<_Ty>::operator=;
		};


		template<class M0VarType>
		class varref_wrapper
		{
		public:
			using type = M0VarType;
			using true_type = M0VarType::type;
			using reference = true_type&;
			using const_reference = const true_type&;
			using pointer = true_type*;
			using const_pointer = const true_type*;

			varref_wrapper() = default;
			varref_wrapper(const std::string& key) noexcept { init(key); }

			bool init(const std::string& key) noexcept
			{
				M0VarType::import_var(&VarPointer, key);
				return VarPointer != nullptr;
			}

			operator var_storage& ()					noexcept { return *VarPointer; }
			operator const var_storage& ()			const noexcept { return *VarPointer; }

			operator reference()					noexcept { return VarPointer->get(); }
			operator const_reference()				const noexcept { return VarPointer->get(); }

			_NODISCARD reference get()				noexcept { return VarPointer->get(); }
			_NODISCARD const_reference get()		const noexcept { return VarPointer->get(); }

			_NODISCARD auto& operator[](size_t i)	noexcept { return VarPointer->get()[i]; }
			const auto& operator[](size_t i)		const noexcept { return VarPointer->get()[i]; }

			_NODISCARD auto data()					noexcept { return VarPointer->data(); }
			_NODISCARD auto data()					const noexcept { return VarPointer->data(); }

			_NODISCARD size_t size()				const noexcept { return VarPointer->size(); }

			auto& operator=(const_reference o) { return VarPointer->operator=(o); }

		private:
			M0VarType* VarPointer{ };
		};


		template<typename _Ty, bool _HasNotifier>				using custom_var = var_wrapper<_Ty, _HasNotifier>;
		template<typename _Ty, size_t _Size, bool _HasNotifier>	using custom_array = var_wrapper<std::array<_Ty, _Size>, _HasNotifier>;
		template<typename _Ty, bool _HasNotifier>				using custom_ref = varref_wrapper<custom_var<_Ty, _HasNotifier>>;
		template<typename _Ty, size_t _Size, bool _HasNotifier>	using custom_array_ref = varref_wrapper<custom_array<_Ty, _Size, _HasNotifier>>;

		using Bool = custom_var<bool, false>;
		using BoolN = custom_var<bool, true>;
		using BoolRef = custom_ref<Bool, false>;
		using BoolRefN = custom_ref<BoolN, true>;

		using Int = custom_var<int, false>;
		using IntN = custom_var<int, true>;
		using IntRef = custom_ref<Int, false>;
		using IntRefN = custom_ref<IntN, true>;

		using Float = custom_var<float, false>;
		using FloatN = custom_var<float, true>;
		using FloatRef = custom_ref<Float, false>;
		using FloatRefN = custom_ref<FloatN, true>;

		using String = custom_var<std::string, false>;
		using StringN = custom_var<std::string, true>;
		using StringRef = custom_ref<String, false>;
		using StringRefN = custom_ref<StringN, true>;

		template<typename _Ty, size_t _Size> using Array = custom_array<_Ty, _Size, false>;
		template<typename _Ty, size_t _Size> using ArrayN = custom_array<_Ty, _Size, true>;
		template<typename _Ty, size_t _Size> using ArrayRef = custom_array_ref<_Ty, _Size, false>;
		template<typename _Ty, size_t _Size> using ArrayRefN = custom_array_ref<_Ty, _Size, true>;



		// Json : [0] = "x", [1] = "y"
		template<typename _Ty, bool _HasNotifier>
		class arrayxy_wrapper final : public custom_array<_Ty, 2, _HasNotifier>
		{
		public:
			using base_type = custom_array<_Ty, 2, _HasNotifier>;
			TCONFIG_INHERIT_FROM(arrayxy_wrapper, typename base_type::type, base_type);

		public:
			using base_type::get;

			_NODISCARD _Ty& x()			  noexcept { return get()[0]; }
			_NODISCARD _Ty x()		const noexcept { return get()[0]; }
			_NODISCARD _Ty& y()			  noexcept { return get()[1]; }
			_NODISCARD _Ty y()		const noexcept { return get()[1]; }

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

		template<typename _Ty>	using ArrayXY = arrayxy_wrapper<_Ty, false>;
		template<typename _Ty>	using ArrayXYN = arrayxy_wrapper<_Ty, true>;
		template<typename _Ty>	using ArrayXYRef = custom_array_ref<arrayxy_wrapper, 2, false>;
		template<typename _Ty>	using ArrayXYRefN = custom_array_ref<arrayxy_wrapper, 2, true>;
	}
}