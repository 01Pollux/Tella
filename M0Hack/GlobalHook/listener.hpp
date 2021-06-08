#pragma once

#include <functional>
#include <variant>
#include <map>
#include <optional>


#define EVENT_NULL_NAME			nullptr

#define EVENT_KEY_LOAD_DLL_EARLY	"LoadDLL_Early"
#define EVENT_KEY_LOAD_DLL			"LoadDLL"
#define EVENT_KEY_UNLOAD_DLL		"UnloadDLL"
#define EVENT_KEY_UNLOAD_DLL_LATE	"UnloadDLL_Late"

#define EVENT_KEY_INIT_MENU		"InitMenu"
#define EVENT_KEY_RENDER_MENU	"RenderMenu"
#define EVENT_KEY_RENDER_EXTRA	"RenderExtra"

#define EVENT_KEY_ENTITY_CREATED "OnEntityCreated"
#define EVENT_KEY_ENTITY_DELETED "OnEntityDeleted"

class M0EventData;
using EventCallback = std::function<void(M0EventData*)>;

using M0EventName = const char*;
using M0EventCustomName = const char*;
constexpr std::nullptr_t M0EventBadData = nullptr;

class M0EventData
{
public:
	using Variant = std::variant<
		bool,
		int8_t, 
		int16_t, 
		int32_t, 
		void*, 
		float, 
		double, 
		std::string
	>;

	void set_bool(const char* key, bool val) noexcept		{ emplace(key, static_cast<int8_t>(val)); }
	void set_int8(const char* key, int8_t val) noexcept		{ emplace(key, val); }
	void set_int16(const char* key, int16_t val) noexcept	{ emplace(key, val); }
	void set_int32(const char* key, int32_t val) noexcept	{ emplace(key, val); }
	void set_uint(const char* key, uint32_t val) noexcept	{ emplace(key, static_cast<int32_t>(val)); }
	void set_int(const char* key, int val) noexcept			{ emplace(key, val); }
	void set_ptr(const char* key, void* val) noexcept		{ emplace(key, val); }
	void set_float(const char* key, float val) noexcept		{ emplace(key, val); }
	void set_double(const char* key, double val) noexcept	{ emplace(key, val); }
	void set_string(const char* key, const std::string& val) noexcept	{ emplace(key, val); }
	void set_string(const char* key, std::string&& val) noexcept		{ emplace(key, std::move(val)); }

	template<typename DataType>
	DataType& alloc(const char* key) noexcept
	{
		return DataMap[key].emplace<DataType>();
	}

	Variant& find(const char* key) 				{ DataMap.find(key)->second; }
	const Variant& find(const char* key) const	{ DataMap.find(key)->second; }

	auto begin() noexcept		 { return DataMap.begin(); }
	auto rbegin() noexcept		 { return DataMap.rbegin(); }
	auto begin() const noexcept  { return DataMap.begin(); }
	auto rbegin() const noexcept { return DataMap.rbegin(); }
	auto cbegin() const noexcept { return DataMap.cbegin(); }
	auto crbegin() const noexcept{ return DataMap.crbegin(); }

	auto end() noexcept			{ return DataMap.end(); }
	auto rend() noexcept		{ return DataMap.rend(); }
	auto end() const noexcept	{ return DataMap.end(); }
	auto rend() const noexcept	{ return DataMap.rend(); }
	auto cend() const noexcept	{ return DataMap.cend(); }
	auto crend() const noexcept	{ return DataMap.crend(); }

	size_t size() const noexcept { return DataMap.size(); }
	bool contains(const char* key) const { return DataMap.contains(key); }
	void erase(const char* key) noexcept { DataMap.erase(key); }

	bool				get_bool(const char* key)	const noexcept { return static_cast<bool>(std::get<int8_t>(find(key))); }
	int8_t				get_int8(const char* key)	const noexcept { return std::get<int8_t>(find(key)); }
	int16_t				get_int16(const char* key)	const noexcept { return std::get<int16_t>(find(key)); }
	int32_t				get_int32(const char* key)	const noexcept { return std::get<int32_t>(find(key)); }
	uint32_t			get_uint(const char* key)	const noexcept { return static_cast<uint32_t>(std::get<int32_t>(find(key))); }
	int					get_int(const char* key)	const noexcept { return std::get<int32_t>(find(key)); }
	const void*			get_ptr(const char* key)	const noexcept { return std::get<void*>(find(key)); }
	float				get_float(const char* key)	const noexcept { return std::get<float>(find(key)); }
	double				get_double(const char* key) const noexcept { return std::get<double>(find(key)); }
	const std::string&	get_string(const char* key) const noexcept { return std::get<std::string>(find(key)); }

private:
	template<typename DataType>
	void emplace(const char* key, const DataType& val)
	{
		DataMap[key].emplace<DataType>(val);
	}
	
	template<typename DataType>
	void emplace(const char* key, DataType&& val)
	{
		DataMap[key] = std::move(val);
	}

	 std::map<const char*, Variant> DataMap;
};


namespace M0EventManager
{
	class IEventWrapper;
	struct EventData
	{
	public:
		using event_iterator = typename std::vector<IEventWrapper>::iterator;
		EventData() = default;
		EventData(event_iterator iter) noexcept : Iterator(iter) { };

		operator bool() const noexcept;
		auto operator->() noexcept		{ return Iterator; }
		auto get() noexcept				{ return Iterator; }
		auto get() const noexcept		{ return Iterator; }

		void operator()(M0EventData* data = M0EventBadData);

	private:
		event_iterator Iterator{ };
	};

	void AddListener(M0EventName event_name, const EventCallback& callback, M0EventCustomName custom_name);
	void RemoveListener(M0EventName event_name, const EventCallback& callback, M0EventCustomName custom_name);

	EventData Find(M0EventName event_name);
	void Destroy(const EventData& event_info);
	void Destroy(M0EventName event_name);
};