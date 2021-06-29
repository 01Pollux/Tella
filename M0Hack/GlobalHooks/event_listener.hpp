#pragma once

#include <variant>
#include <map>
#include <string>
#include <vector>
#include <functional>

namespace tella
{
	namespace event_listener
	{
		class data
		{
		public:
			using variant = std::variant<
				bool,
				int8_t,
				int16_t,
				int32_t,
				void*,
				float,
				double,
				std::string
			>;

			void set_bool(const char* key, bool val)					{ emplace(key, static_cast<int8_t>(val)); }
			void set_int8(const char* key, int8_t val)					{ emplace(key, val); }
			void set_int16(const char* key, int16_t val)				{ emplace(key, val); }
			void set_int32(const char* key, int32_t val)				{ emplace(key, val); }
			void set_uint(const char* key, uint32_t val)				{ emplace(key, static_cast<int32_t>(val)); }
			void set_int(const char* key, int val)						{ emplace(key, val); }
			void set_ptr(const char* key, void* val)					{ emplace(key, val); }
			void set_float(const char* key, float val)					{ emplace(key, val); }
			void set_double(const char* key, double val)				{ emplace(key, val); }
			void set_string(const char* key, const std::string& val)	{ emplace(key, val); }
			void set_string(const char* key, std::string&& val)			{ emplace(key, std::move(val)); }

			template<typename DataType>
			DataType& alloc(const char* key)							{ return DataMap[key].emplace<DataType>(); }

			variant* find(const char* key)								{ auto iter = DataMap.find(key); iter == DataMap.end() ? nullptr : iter->second; }
			const variant* find(const char* key) const					{ auto iter = DataMap.find(key); iter == DataMap.end() ? nullptr : iter->second; }

			auto begin()		  noexcept { return DataMap.begin(); }
			auto rbegin()		  noexcept { return DataMap.rbegin(); }
			auto begin()	const noexcept { return DataMap.begin(); }
			auto rbegin()	const noexcept { return DataMap.rbegin(); }
			auto cbegin()	const noexcept { return DataMap.cbegin(); }
			auto crbegin()	const noexcept { return DataMap.crbegin(); }

			auto end()			  noexcept { return DataMap.end(); }
			auto rend()			  noexcept { return DataMap.rend(); }
			auto end()		const noexcept { return DataMap.end(); }
			auto rend()		const noexcept { return DataMap.rend(); }
			auto cend()		const noexcept { return DataMap.cend(); }
			auto crend()	const noexcept { return DataMap.crend(); }

			size_t size()	const noexcept { return DataMap.size(); }
			bool contains(const char* key) 
							const		  { return DataMap.contains(key); }
			void erase(const char* key)	  { DataMap.erase(key); }

			bool				get_bool(const char* key)	const { return static_cast<bool>(std::get<int8_t>(*find(key))); }
			int8_t				get_int8(const char* key)	const { return std::get<int8_t>(*find(key)); }
			int16_t				get_int16(const char* key)	const { return std::get<int16_t>(*find(key)); }
			int32_t				get_int32(const char* key)	const { return std::get<int32_t>(*find(key)); }
			uint32_t			get_uint(const char* key)	const { return static_cast<uint32_t>(std::get<int32_t>(*find(key))); }
			int					get_int(const char* key)	const { return std::get<int32_t>(*find(key)); }
			const void*			get_ptr(const char* key)	const { return std::get<void*>(*find(key)); }
			float				get_float(const char* key)	const { return std::get<float>(*find(key)); }
			double				get_double(const char* key) const { return std::get<double>(*find(key)); }
			const std::string& get_string(const char* key)	const { return std::get<std::string>(*find(key)); }

		private:
			template<typename DataType>	void emplace(const char* key, const DataType& val)	 { DataMap[key].emplace<DataType>(val); }
			template<typename DataType>	void emplace(const char* key, DataType&& val)		 { DataMap[key] = std::move(val); }

			std::map<const char*, variant> DataMap;
		};

		using callback_type = std::function<void(data*)>;

		class event_wrapper;

		struct callback_data
		{
		public:
			using event_iterator = typename std::vector<event_wrapper>::iterator;
			callback_data() = default;
			callback_data(event_iterator iter) noexcept : _Iterator(iter) { };

			operator bool()			const noexcept;
			auto operator->()			  noexcept { return _Iterator; }
			auto get()					  noexcept { return _Iterator; }
			auto get()				const noexcept { return _Iterator; }

			void operator()(data* data = nullptr);

		private:
			event_iterator _Iterator{ };
		};


		void insert(const char* event_name, const callback_type& callback, const char* custom_name);
		void remove(const char* event_name, const callback_type& callback);

		callback_data find(const char* event_name);
		void destroy(const callback_data& event_info);
		void destroy(const char* event_name);

		namespace names
		{
			constexpr const char* Null				= nullptr;

			constexpr const char* LoadDLL_Early		= "LoadDLL_Early";
			constexpr const char* LoadDLL			= "LoadDLL";
			constexpr const char* UnLoadDLL			= "UnloadDLL";
			constexpr const char* UnLoadDLL_Late	= "UnloadDLL_Late";

			constexpr const char* InitImGuiMenu		= "InitMenu";
			constexpr const char* RenderImGui		= "RenderImGui";
			constexpr const char* RenderExtra		= "RenderExtra";

			constexpr const char* EntityCreated		= "OnEntityCreated";
			constexpr const char* EntityRemoved		= "OnEntityDeleted";
		}
	}
}

