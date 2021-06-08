#pragma once

#include <type_traits>

#define ECLASS_BITWISE_OPERATORS(ENUMCLASS) \
 \
template <class IntType, std::enable_if_t<std::is_integral_v<IntType>, int> = 0> \
[[nodiscard]] constexpr ENUMCLASS operator<<(const ENUMCLASS l, const IntType shift) noexcept \
{ \
    using TrueType = std::underlying_type_t<ENUMCLASS>; \
    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) << shift)); \
} \
 \
template <class IntType, std::enable_if_t<std::is_integral_v<IntType>, int> = 0> \
[[nodiscard]] constexpr ENUMCLASS operator>>(const ENUMCLASS l, const IntType shift) noexcept \
{  \
    using TrueType = std::underlying_type_t<ENUMCLASS>; \
    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) >> shift)); \
} \
 \
[[nodiscard]] constexpr ENUMCLASS operator|(const ENUMCLASS l, const ENUMCLASS r) noexcept \
{ \
    using TrueType = std::underlying_type_t<ENUMCLASS>; \
    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) | static_cast<unsigned int>(r))); \
} \
 \
constexpr ENUMCLASS operator|=(ENUMCLASS& l, const ENUMCLASS r) noexcept \
{ \
    return l = l | r; \
} \
 \
[[nodiscard]] constexpr ENUMCLASS operator&(const ENUMCLASS l, const ENUMCLASS r) noexcept \
{ \
    using TrueType = std::underlying_type_t<ENUMCLASS>; \
    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) & static_cast<unsigned int>(r))); \
} \
constexpr ENUMCLASS operator&=(ENUMCLASS& l, const ENUMCLASS r) noexcept \
{ \
    return l = l & r; \
} \
\
[[nodiscard]] constexpr ENUMCLASS operator^(const ENUMCLASS l, const ENUMCLASS r) noexcept \
{ \
    \
    using TrueType = std::underlying_type_t<ENUMCLASS>; \
    return static_cast<ENUMCLASS>(static_cast<TrueType>(static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r))); \
} \
 \
constexpr ENUMCLASS operator^=(ENUMCLASS& l, const ENUMCLASS r) noexcept \
{ \
    return l = l ^ r; \
} \
 \
[[nodiscard]] constexpr bool operator!(const ENUMCLASS l) noexcept \
{ \
    using TrueType = std::underlying_type_t<ENUMCLASS>; \
    return !static_cast<TrueType>(l); \
} \

template<typename _Ty>
constexpr bool HasBitSet(const _Ty& l, const _Ty& r)
{
    return (static_cast<unsigned int>(l) & static_cast<unsigned int>(r)) == static_cast<unsigned int>(r);
}

// TODO bitset template + enum class