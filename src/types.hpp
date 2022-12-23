#pragma once

#include <cstddef>
#include <cstdint>
#include <tl/expected.hpp>
#include <tl/optional.hpp>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

template<typename V, typename E>
using Result = tl::expected<V, E>;

template<typename E>
using Error = tl::unexpected<E>;

template<typename V>
using Optional = tl::optional<V>;

inline constexpr usize WordSize = 4;
inline constexpr usize HalfwordSize = 2;
