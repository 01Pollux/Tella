#pragma once

#include <forward_list>
#include <list>
#include <unordered_map>
#include <assert.h>

template<typename T>
class AutoList
{
	using _CType = T*;
public:
	AutoList()
	{
		_List.push_front(static_cast<_CType>(this));
	}

	virtual ~AutoList()
	{
		_List.remove(static_cast<_CType>(this));
	}

	static const std::forward_list<_CType>& List()
	{
		return _List;
	}

private:

	static inline std::forward_list<_CType> _List;
};


template<typename T, typename K = const char*>
class AutoHashMap
{
public:
	AutoHashMap(K key): hash_key(key)
	{
		T* t = static_cast<T*>(this);

		AllocHashMap();
	}
	virtual ~AutoHashMap()
	{
		if (m_HashMap)
		{
			T* t = static_cast<T*>(this);
			assert(m_HashMap->erase(this->hash_key) == 1);

			if (m_HashMap->empty())
			{
				delete m_HashMap;
				m_HashMap = nullptr;
			}
		}
	}

	static const std::unordered_map<K, T*>& MultiMap()
	{
		AllocHashMap();
		return *m_HashMap;
	}

private:
	static void AllocHashMap()
	{
		if (!m_HashMap) {
			m_HashMap = new std::unordered_map<K, T*>();
		}
	}

	K hash_key;
	static inline std::unordered_map<K, T*>* m_HashMap = nullptr;
};
