#pragma once

#include <vector>
#include <memory>


template<typename _Ty>
struct ValveUtlMemory
{
	std::unique_ptr<_Ty[]> Data;
	int Capacity{ };
	int GrowSize{ };
};

template<class _Ty>
class ValveUtlVector
{
	using _AllocTy = std::allocator<_Ty>;
	using _Alloc = std::allocator_traits<_AllocTy>;

public:
	// constructor, destructor
	ValveUtlVector(int growSize = 0, int initSize = 0);
	ValveUtlVector(_Ty* pMemory, int allocationCount, int numElements = 0);

	// element access
	_Ty& operator[](int i) noexcept;
	const _Ty& operator[](int i) const noexcept;
	_Ty& Head() noexcept;
	const _Ty& Head() const noexcept;
	_Ty& Tail() noexcept;
	const _Ty& Tail() const noexcept;

	// Gets the base address (can change when adding elements!)
	_Ty* Base() noexcept { return Storage.Data.get(); }
	const _Ty* Base() const noexcept { return Storage.Data.get(); }

	// Returns the number of elements in the vector
	int Count() const noexcept;

	// Is element index valid?
	bool IsValidIndex(int i) const noexcept { return i >= 0 && i < Count(); }
	static int InvalidIndex() noexcept { return -1; } 

	// Adds an element, uses default constructor
	int AddToHead() noexcept;
	int AddToTail() noexcept;
	int InsertBefore(int elem) noexcept;
	int InsertAfter(int elem) noexcept;

	// Adds an element, uses copy constructor
	int AddToHead(const _Ty& src) noexcept;
	int AddToTail(const _Ty& src) noexcept;
	int InsertBefore(int elem, const _Ty& src) noexcept;
	int InsertAfter(int elem, const _Ty& src) noexcept;

	// Adds multiple elements, uses default constructor
	int AddMultipleToHead(int num) noexcept;
	int AddMultipleToTail(int num, const _Ty* pToCopy = nullptr) noexcept;
	int InsertMultipleBefore(int elem, int num, const _Ty* pToCopy = nullptr) noexcept;	// If pToCopy is set, then it's an array of length 'num' and
	int InsertMultipleAfter(int elem, int num) noexcept;

	// Calls RemoveAll() then AddMultipleToTail.
	void SetSize(int size) noexcept;
	void SetCount(int count) noexcept;

	// Calls SetSize and copies each element.
	void CopyArray(const _Ty* pArray, int size) noexcept;

	// Add the specified array to the tail.
	int AddVectorToTail(const ValveUtlVector<_Ty>& src) noexcept;

	// Finds an element (element needs operator== defined)
	int Find(const _Ty& src) const noexcept;

	bool HasElement(const _Ty& src) const noexcept;

	// Makes sure we have enough memory allocated to store a requested # of elements
	void EnsureCapacity(int num) noexcept;

	// Element removal
	void FastRemove(int elem) noexcept;	// doesn't preserve order
	void Remove(int elem) noexcept;		// preserves order, shifts elements
	bool FindAndRemove(const _Ty& src) noexcept;	// removes first occurrence of src, preserves order, shifts elements
	void RemoveMultiple(int elem, int num) noexcept;	// preserves order, shifts elements
	void RemoveAll() noexcept;				// doesn't deallocate memory

	// Memory deallocation
	void Purge() noexcept;

	// Purges the list and calls delete on each element in it.
	void PurgeAndDeleteElements() noexcept;

	// Set the size by which it grows when it needs to allocate more memory.
	void SetGrowSize(int size) noexcept { Storage.GrowSize = size; }

	int NumAllocated() const noexcept;	// Only use this if you really know what you're doing!

	ValveUtlVector(const ValveUtlVector&) = delete;
	ValveUtlVector& operator=(const ValveUtlVector&) noexcept;
	ValveUtlVector(ValveUtlVector&&) = default;
	ValveUtlVector& operator=(ValveUtlVector&&) = default;

private:
	void Alloc(int growSize, int initSize) noexcept
	{
		if (initSize)
		{
			Storage.Data = std::make_unique<_Ty[]>(static_cast<size_t>(initSize));
			Storage.Capacity = initSize;
			Storage.GrowSize = growSize;
		}
	}

	// Grows the vector
	void GrowVector(int num = 1) noexcept;

	// Shifts elements....
	void ShiftElementsRight(int elem, int num = 1) noexcept;
	void ShiftElementsLeft(int elem, int num = 1) noexcept;

	ValveUtlMemory<_Ty> Storage;
	int Size;

	_Ty* Elements;

	inline void ResetDbgInfo() noexcept
	{
		Elements = Base();
	}
};


template<typename _Ty>
inline 
ValveUtlVector<_Ty>::ValveUtlVector(int growSize, int initSize) :
	Size(0)
{
	Alloc(growSize, initSize);
	ResetDbgInfo();
}

template<typename _Ty>
inline 
ValveUtlVector<_Ty>::ValveUtlVector(_Ty* pMemory, int allocationCount, int numElements) :
	Size(numElements)
{
	Storage.Data = pMemory;
	Storage.Capacity = allocationCount;
	ResetDbgInfo();
}


template<typename _Ty>
inline ValveUtlVector<_Ty>& 
ValveUtlVector<_Ty>::operator=(const ValveUtlVector<_Ty>& other) noexcept
{
	int nCount = other.Count();
	SetSize(nCount);
	std::memcpy(Storage.Data.get(), other.Storage.Data.get(), nCount * sizeof(_Ty));
	return *this;
}


template<typename _Ty>
inline _Ty&
ValveUtlVector<_Ty>::operator[](int i) noexcept
{
	return Storage.Data[i];
}

template<typename _Ty>
inline const _Ty& 
ValveUtlVector<_Ty>::operator[](int i) const noexcept
{
	return Storage.Data[i];
}

template<typename _Ty>
inline _Ty&
ValveUtlVector<_Ty>::Head() noexcept
{
	return *Storage.Data;
}

template<typename _Ty>
inline const _Ty&
ValveUtlVector<_Ty>::Head() const noexcept
{
	return *Storage.Data;
}

template<typename _Ty>
inline _Ty&
ValveUtlVector<_Ty>::Tail() noexcept
{
	return Storage.Data[Size - 1];
}

template<typename _Ty>
inline const _Ty&
ValveUtlVector<_Ty>::Tail() const noexcept
{
	return Storage.Data[Size - 1];
}

template<typename _Ty>
inline int
ValveUtlVector<_Ty>::Count() const noexcept
{
	return Size;
}

//-----------------------------------------------------------------------------
// Grows the vector
//-----------------------------------------------------------------------------
template<typename _Ty>
void 
ValveUtlVector<_Ty>::GrowVector(int num) noexcept
{
	if (Size + num > Storage.Capacity)
		Alloc(Storage.GrowSize, Size + num);

	Size += num;
	ResetDbgInfo();
}


template<typename _Ty>
void
ValveUtlVector<_Ty>::EnsureCapacity(int num) noexcept
{
	Alloc(Storage.GrowSize, num);
	ResetDbgInfo();
}


template<typename _Ty>
void 
ValveUtlVector<_Ty>::ShiftElementsRight(int elem, int num) noexcept
{
	int numToMove = Size - elem - num;
	if ((numToMove > 0) && (num > 0))
		memmove(&this->operator[](elem + num), &this->operator[](elem), numToMove * sizeof(_Ty));
}

template<typename _Ty>
void 
ValveUtlVector<_Ty>::ShiftElementsLeft(int elem, int num) noexcept
{
	int numToMove = Size - elem - num;
	if ((numToMove > 0) && (num > 0))
	{
		memmove(&this->operator[](elem), &this->operator[](elem + num), numToMove * sizeof(_Ty));
	}
}


template<typename _Ty>
inline int 
ValveUtlVector<_Ty>::AddToHead() noexcept
{
	return InsertBefore(0);
}

template<typename _Ty>
inline int 
ValveUtlVector<_Ty>::AddToTail() noexcept
{
	return InsertBefore(Size);
}

template<typename _Ty>
inline int ValveUtlVector<_Ty>::InsertAfter(int elem) noexcept
{
	return InsertBefore(elem + 1);
}

template<typename _Ty>
int ValveUtlVector<_Ty>::InsertBefore(int elem) noexcept
{
	_AllocTy al;
	GrowVector();
	ShiftElementsRight(elem);
	_Alloc::construct(al, &this->operator[](elem));
	return elem;
}


template<typename _Ty>
inline int 
ValveUtlVector<_Ty>::AddToHead(const _Ty& src) noexcept
{
	return InsertBefore(0, src);
}

template<typename _Ty>
inline int 
ValveUtlVector<_Ty>::AddToTail(const _Ty& src) noexcept
{
	return InsertBefore(Size, src);
}

template<typename _Ty>
inline int 
ValveUtlVector<_Ty>::InsertAfter(int elem, const _Ty& src) noexcept
{
	return InsertBefore(elem + 1, src);
}

template<typename _Ty>
int ValveUtlVector<_Ty>::InsertBefore(int elem, const _Ty& src) noexcept
{
	_AllocTy al;
	GrowVector();
	ShiftElementsRight(elem);
	_Alloc::construct(al, &this->operator[](elem), src);
	return elem;
}


//-----------------------------------------------------------------------------
// Adds multiple elements, uses default constructor
//-----------------------------------------------------------------------------
template<typename _Ty>
inline int ValveUtlVector<_Ty>::AddMultipleToHead(int num) noexcept
{
	return InsertMultipleBefore(0, num);
}

template<typename _Ty>
inline int ValveUtlVector<_Ty>::AddMultipleToTail(int num, const _Ty* pToCopy) noexcept
{
	return InsertMultipleBefore(Size, num, pToCopy);
}

template<typename _Ty>
int ValveUtlVector<_Ty>::InsertMultipleAfter(int elem, int num) noexcept
{
	return InsertMultipleBefore(elem + 1, num);
}


template<typename _Ty>
void ValveUtlVector<_Ty>::SetCount(int count) noexcept
{
	RemoveAll();
	AddMultipleToTail(count);
}

template<typename _Ty>
inline void ValveUtlVector<_Ty>::SetSize(int size) noexcept
{
	SetCount(size);
}

template<typename _Ty>
void ValveUtlVector<_Ty>::CopyArray(const _Ty* pArray, int size) noexcept
{
	memcpy(Storage.Data.get(), pArray, size * sizeof(_Ty));
}


template<typename _Ty>
int ValveUtlVector<_Ty>::AddVectorToTail(ValveUtlVector const& src) noexcept
{
	Assert(&src != this);

	int base = Count();

	// Make space.
	AddMultipleToTail(src.Count());

	// Copy the elements.	
	for (int i = 0; i < src.Count(); i++)
	{
		(*this)[base + i] = src[i];
	}

	return base;
}

template<typename _Ty>
inline int 
ValveUtlVector<_Ty>::InsertMultipleBefore(int elem, int num, const _Ty* pToInsert) noexcept
{
	if (num == 0)
		return elem;

	GrowVector(num);
	ShiftElementsRight(elem, num);

	// Invoke default constructors
	_AllocTy al{ };
	for (int i = 0; i < num; ++i)
		_Alloc::construct(al, &this->operator[](elem + i));

	// Copy stuff in?
	if (pToInsert)
	{
		for (int i = 0; i < num; i++)
		{
			this->operator[](elem + i) = pToInsert[i];
		}
	}

	return elem;
}


//-----------------------------------------------------------------------------
// Finds an element (element needs operator== defined)
//-----------------------------------------------------------------------------
template<typename _Ty>
int ValveUtlVector<_Ty>::Find(const _Ty& src) const noexcept
{
	for (int i = 0; i < Count(); ++i)
	{
		if (this->operator[](i) == src)
			return i;
	}
	return -1;
}

template<typename _Ty>
bool ValveUtlVector<_Ty>::HasElement(const _Ty& src) const noexcept
{
	return (Find(src) >= 0);
}


//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------
template<typename _Ty>
void ValveUtlVector<_Ty>::FastRemove(int elem) noexcept
{
	_AllocTy al;
	_Alloc::destroy(al, &this->operator[](elem));
	if (Size > 0)
	{
		memcpy(&this->operator[](elem), &this->operator[](Size - 1), sizeof(_Ty));
		--Size;
	}
}

template<typename _Ty>
void ValveUtlVector<_Ty>::Remove(int elem) noexcept
{
	_AllocTy al;
	_Alloc::destroy(al, &this->operator[](elem));
	ShiftElementsLeft(elem);
	--Size;
}

template<typename _Ty>
bool ValveUtlVector<_Ty>::FindAndRemove(const _Ty& src) noexcept
{
	int elem = Find(src);
	if (elem != -1)
	{
		Remove(elem);
		return true;
	}
	return false;
}

template<typename _Ty>
void ValveUtlVector<_Ty>::RemoveMultiple(int elem, int num) noexcept
{
	_AllocTy al;
	for (int i = elem + num; --i >= elem; )
		_Alloc::destroy(al, &this->operator[](i));

	ShiftElementsLeft(elem, num);
	Size -= num;
}

template<typename _Ty>
void ValveUtlVector<_Ty>::RemoveAll() noexcept
{
	_AllocTy al;
	for (int i = 0; i < Size; i++)
		_Alloc::destroy(al, &this->operator[](i));

	Size = 0;
}


//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template<typename _Ty>
inline void ValveUtlVector<_Ty>::Purge() noexcept
{
	RemoveAll();
	Storage.Data = nullptr;
	ResetDbgInfo();
}


template<typename _Ty>
inline void ValveUtlVector<_Ty>::PurgeAndDeleteElements() noexcept
{
	for (int i = 0; i < Size; i++)
	{
		delete this->operator[](i);
	}
	Purge();
}

template<typename _Ty>
inline int ValveUtlVector<_Ty>::NumAllocated() const noexcept
{
	return Storage.Capacity();
}
