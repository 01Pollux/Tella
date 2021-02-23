#pragma once

#include <forward_list>
#include <unordered_map>

template<typename ClassType>
class IAutoList
{
	using IList = std::forward_list<ClassType*>;

public:
	IAutoList()
	{
		vec.push_front(static_cast<ClassType*>(this));
	}

	virtual ~IAutoList()
	{
		vec.remove(static_cast<ClassType*>(this));
	}

	static const IList& List()
	{
		return vec;
	}

	IAutoList(IAutoList&)					= delete;
	IAutoList operator=(const IAutoList&)	= delete;
	IAutoList(IAutoList&&)					= delete;
	IAutoList operator=(IAutoList&&)		= delete;

private:
	static inline IList vec;
};
