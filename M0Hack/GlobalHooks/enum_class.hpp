#pragma once

#include <type_traits>
#include <bitset>

//
//#define ECLASS_BITWISE_OPERATORS(ENUMCLASS) \
// \
//template <class IntType, std::enable_if_t<std::is_integral_v<IntType>, int> = 0> \
//[[nodiscard]] constexpr ENUMCLASS operator<<(const ENUMCLASS l, const IntType shift) noexcept \
//{ \
//    using TrueType = std::underlying_type_t<ENUMCLASS>; \
//    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) << shift)); \
//} \
// \
//template <class IntType, std::enable_if_t<std::is_integral_v<IntType>, int> = 0> \
//[[nodiscard]] constexpr ENUMCLASS operator>>(const ENUMCLASS l, const IntType shift) noexcept \
//{  \
//    using TrueType = std::underlying_type_t<ENUMCLASS>; \
//    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) >> shift)); \
//} \
// \
//[[nodiscard]] constexpr ENUMCLASS operator|(const ENUMCLASS l, const ENUMCLASS r) noexcept \
//{ \
//    using TrueType = std::underlying_type_t<ENUMCLASS>; \
//    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) | static_cast<unsigned int>(r))); \
//} \
// \
//constexpr ENUMCLASS operator|=(ENUMCLASS& l, const ENUMCLASS r) noexcept \
//{ \
//    return l = l | r; \
//} \
// \
//[[nodiscard]] constexpr ENUMCLASS operator&(const ENUMCLASS l, const ENUMCLASS r) noexcept \
//{ \
//    using TrueType = std::underlying_type_t<ENUMCLASS>; \
//    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) & static_cast<unsigned int>(r))); \
//} \
//constexpr ENUMCLASS operator&=(ENUMCLASS& l, const ENUMCLASS r) noexcept \
//{ \
//    return l = l & r; \
//} \
//\
//[[nodiscard]] constexpr ENUMCLASS operator^(const ENUMCLASS l, const ENUMCLASS r) noexcept \
//{ \
//    \
//    using TrueType = std::underlying_type_t<ENUMCLASS>; \
//    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r))); \
//} \
// \
//constexpr ENUMCLASS operator^=(ENUMCLASS& l, const ENUMCLASS r) noexcept \
//{ \
//    return l = l ^ r; \
//} \
// \
//[[nodiscard]] constexpr bool operator!(const ENUMCLASS l) noexcept \
//{ \
//    using TrueType = std::underlying_type_t<ENUMCLASS>; \
//    return !static_cast<TrueType>(l); \
//} \
//
//template<typename _Ty>
//constexpr bool HasBitSet(const _Ty& l, const _Ty& r)
//{
//    return (static_cast<unsigned int>(l) & static_cast<unsigned int>(r)) == static_cast<unsigned int>(r);
//}

namespace bitmask
{
    template<typename _ETy>
    class mask
    {
    public:
        using type = _ETy;
        using bitset_type = std::bitset<static_cast<size_t>(_ETy::_Highest_Enum)>;
        using bitset_reference = bitset_type::reference;

    public:
        constexpr mask() = default;
        constexpr mask(uint64_t val) noexcept : bitset(val) { }
        constexpr mask(uint32_t val) noexcept : bitset(static_cast<uint64_t>(val)) { }

        template <class _Ty, class _Tr, class _Alloc>
        explicit mask(const std::basic_string<_Ty, _Tr, _Alloc>& str,
            typename std::basic_string<_Ty, _Tr, _Alloc>::size_type pos = 0,
            typename std::basic_string<_Ty, _Tr, _Alloc>::size_type count = std::basic_string<_Ty, _Tr, _Alloc>::npos,
            _Ty zero = static_cast<_Ty>('0'), _Ty one = static_cast<_Ty>('1')) : bitset(str, pos, count, zero, one) { }

        template <class _Ty>
        explicit mask(const _Ty* ntcts, typename std::basic_string<_Ty>::size_type count = std::basic_string<_Ty>::npos,
            _Ty zero = static_cast<_Ty>('0'), _Ty one = static_cast<_Ty>('1')) : bitset(ntcts, count, zero, one) { }


        mask& set()                          noexcept { bitset.set();                            return *this; }
        mask& set(type pos, bool val = true)          { bitset.set(static_cast<size_t>(pos), val);return *this;}
        _NODISCARD mask& flip()              noexcept { bitset.flip();                           return *this; }
        _NODISCARD mask& flip(type pos)               { bitset.flip(static_cast<size_t>(pos));   return *this; }
        _NODISCARD mask& reset()             noexcept { bitset.reset();                          return *this; }
        _NODISCARD mask& reset(type pos)              { bitset.reset(static_cast<size_t>(pos));  return *this; }

        _NODISCARD bool test(type pos)      const          { return bitset.test(static_cast<size_t>(pos)); }
        _NODISCARD bool any()               const noexcept { return bitset.any(); }
        _NODISCARD bool none()              const noexcept { return bitset.none(); }
        _NODISCARD bool all()               const noexcept { return bitset.all(); }

        _NODISCARD size_t size()             const noexcept { return bitset.size(); }
        _NODISCARD size_t count()            const noexcept { return bitset.count(); }
        _NODISCARD bool operator[](type pos) const          { return bitset.operator[](static_cast<size_t>(pos)); }
        _NODISCARD auto operator[](type pos)                { return bitset.operator[](static_cast<size_t>(pos)); }
        _NODISCARD bool operator[](size_t pos)const         { return bitset.operator[](pos); }
        _NODISCARD auto operator[](size_t pos)              { return bitset.operator[](pos); }

        template<class _Ty = char, class _Tr = std::char_traits<_Ty>, class _Alloc = std::allocator<_Ty>>
        _NODISCARD auto to_string(_Ty zero = static_cast<_Ty>('0'), _Ty one = static_cast<_Ty>('1'))
                                            const { return bitset.to_string(zero, one); }
        _NODISCARD uint64_t to_ullong()     const { return bitset.to_ullong(); }
        _NODISCARD uint32_t to_ulong()      const { return bitset.to_ulong(); }

        _NODISCARD bitset_type& get()                     noexcept { return bitset; }
        _NODISCARD operator bitset_type& ()               noexcept { return bitset; }
        _NODISCARD const bitset_type& get()         const noexcept { return bitset; }
        _NODISCARD operator const bitset_type& ()   const noexcept { return bitset; }

        _NODISCARD mask operator~()              const noexcept { return ebitset(bitset.operator~()); }
        _NODISCARD mask operator<<(type pos)     const noexcept { return ebitset(bitset.operator<<(static_cast<size_t>(pos))); }
        _NODISCARD mask operator>>(type pos)     const noexcept { return ebitset(bitset.operator>>(static_cast<size_t>(pos))); }
        _NODISCARD bool operator==(type pos)     const noexcept { return bitset.operator==(static_cast<size_t>(pos)); }

        mask& operator>>=(type pos)                   noexcept { bitset.operator>>=(static_cast<size_t>(pos)); return *this; }
        mask& operator<<=(type pos)                   noexcept { bitset.operator<<=(static_cast<size_t>(pos)); return *this; }
        mask& operator^=(const bitset_type& right)    noexcept { bitset.operator^=(right); return *this; }
        mask& operator^=(const mask& right)           noexcept { operator^=(right.get());  return *this; }
        mask& operator|=(const bitset_type& right)    noexcept { bitset.operator|=(right); return *this; }
        mask& operator|=(const mask& right)           noexcept { operator|=(right.get());  return *this; }
        mask& operator&=(const bitset_type& right)    noexcept { bitset.operator&=(right); return *this; }
        mask& operator&=(const mask& right)           noexcept { operator&=(right.get());  return *this; }

        _NODISCARD bool test(const mask& other) const
        {
            if (other.size() != size())
                return false;

            for (size_t i = 0, c = other.count(); i < size() && c > 0; ++i)
            {
                if (other[i])
                {
                    if (!bitset[i])
                        return false;
                    --c;
                }
            }

            return true;
        }

    private:
        bitset_type bitset;
    };

    template<typename _ETy>
    _NODISCARD mask<_ETy> operator&(const mask<_ETy>& l, const mask<_ETy>& r) noexcept
    {
        auto ret = l;
        return ret &= r;
    }

    template<typename _ETy>
    _NODISCARD mask<_ETy> operator|(const mask<_ETy>& l, const mask<_ETy>& r) noexcept
    {
        auto ret = l;
        return ret |= r;
    }

    template<typename _ETy>
    _NODISCARD mask<_ETy> operator^(const mask<_ETy>& l, const mask<_ETy>& r) noexcept
    {
        auto ret = l;
        return ret ^= r;
    }

    template<typename _ETy0>
    constexpr uint64_t to_mask(_ETy0 first)
    {
        return 1Ui64 << static_cast<uint64_t>(first);
    }
   
    template<typename _ETy0, typename... _ETy1>
    constexpr uint64_t to_mask(_ETy0 first, _ETy1... vals)
    {
        return (1Ui64 << static_cast<uint64_t>(first)) | to_mask(vals...);
    }
}
