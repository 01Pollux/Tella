#pragma once

#include "Cheats/Main.hpp"
#include "esp_reserved.hpp"
#include "Helper/DrawTools.hpp"
#include "Hooks/PaintTraverse.hpp"

#include <type_traits>
#include "Helper/Format.hpp"

enum class ESPType : char8_t
{
	Player,
	Building,
	Objects,

	ESPMAX
};

struct ESPData
{
public:
	IBaseEntity Entity;
	float Distance;
	std::vector<std::wstring> Strings;

	ESPType Type = ESPType::Objects;

	template<class _WStringTy, 
		std::enable_if_t<std::is_same_v<typename _WStringTy::value_type, wchar_t>, int> = 0>
	void AddEntityString(bool state, const _WStringTy& str)
	{
		if (state)
			Strings.emplace_back(str);
	}

	template<class _WStringTy,
		std::enable_if_t<std::is_same_v<typename _WStringTy::value_type, wchar_t>, int> = 0>
	void AddEntityString(bool state, const _WStringTy& str, std::vector<std::wstring>::const_iterator iter)
	{
		if (state)
			Strings.emplace(iter, str);
	}

	void AddEntityString(bool state, const std::string& str)
	{
		if (state)
			Strings.push_back(StringTransform<std::wstring>(str));
	}
	
	void AddEntityString(bool state, const std::string& str, std::vector<std::wstring>::const_iterator iter)
	{
		if (state)
			Strings.emplace(iter, StringTransform<std::wstring>(str));
	}
};

struct ESPBoxInfo
{
	static constexpr int UPPER_L = 0;
	static constexpr int UPPER_R = 1;
	static constexpr int BOTTOM_R = 2;
	static constexpr int BOTTOM_L = 3;

	enum class PosType
	{
		Bottom,
		Upper
	};

	template<PosType type>
	const DrawTools::ScreenCoord& GetPosition(int offset) const noexcept
	{
		if constexpr (type == PosType::Bottom)
			return corners[offset];
		else 
			return corners[offset + 4];
	}

	std::pair<DrawTools::ScreenCoord, DrawTools::ScreenCoord> 
		GetMinMax() const noexcept
	{
		DrawTools::ScreenCoord max{ -1, -1 }, min{ 65536, 65536 };

		for (auto [x, y] : corners)
		{
			if (x > max.x)
				max.x = x;
			if (x < min.x)
				min.x = x;

			if (y > max.y)
				max.y = y;
			if (y < min.y)
				min.y = y;
		}

		return { min, max };
	}

	DrawTools::ScreenCoord corners[8];
};

class IESPHack
{
public:
	IESPHack();

private:
	void OnRender();
	HookRes OnPaintTraverse(uint32_t);

private:
	ESPInfo::Player		PlayerESPInfo[ESPInfo::MAX_TEAMS]{ true, false };
	ESPInfo::Building	BuildingESPInfo[ESPInfo::MAX_TEAMS]{ true, false };
	ESPInfo::Object		ObjectESPInfo;

	std::vector<ESPData> DrawData;
	
	void ProcessPlayer_FirstPass(const ITFPlayer, const Vector& my_pos);
	void ProcessPlayer_SecondPass(const ITFPlayer, float dist);

	void ProcessBuilding(const EntClassID, const IBaseObject, const float dist);
	void ProcessObject(const EntClassID, const IBaseEntity, const float dist);

	void ProcessEntity(const IBaseEntity, const Vector& my_pos);

	void CollectEntities();

	bool GetBoxInfo(const ESPData* info, ESPBoxInfo* boxinfo);
	void DrawFromData(const ESPData* info);

	bool IsAnyActive() noexcept
	{
		for (std::reference_wrapper<ESPInfo::Shared> infos[]
			{
				PlayerESPInfo[0],
				PlayerESPInfo[1],
				BuildingESPInfo[0],
				BuildingESPInfo[1],
				ObjectESPInfo
			};
			auto& v : infos)
		{
			if (v.get().Enable)
				return true;
		}

		return false;
	}
};