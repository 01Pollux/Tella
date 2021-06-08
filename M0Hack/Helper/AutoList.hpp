#pragma once

#include <vector>
#include <memory>

template<typename ClassType>
class IAutoList
{
	using StorageType = std::vector<ClassType*>;
public:
	IAutoList()
	{
		if (!Entries)
			Init();
		Entries->emplace_back(static_cast<ClassType*>(this));
	}

	virtual ~IAutoList()
	{
		if (Entries)
		{
			Entries->erase(std::find(Entries->cbegin(), Entries->cend(), static_cast<ClassType*>(this)));
			if (Entries->empty())
				Entries = nullptr;
		}
	}

	static const auto& GetEntries() noexcept
	{
		return *Entries;
	}

	static void RemoveAll() noexcept
	{
		Entries = nullptr;
	}

	IAutoList(IAutoList&)					= delete;
	IAutoList& operator=(const IAutoList&)	= delete;
	IAutoList(IAutoList&&)					= delete;
	IAutoList& operator=(IAutoList&&)		= delete;
private:
	static void Init()
	{
		Entries = std::make_unique<StorageType>();
	}

	static inline std::unique_ptr<StorageType> Entries;
};