// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// Include kuku/config.h only on platforms with CMake configuration (not MSVC).
#ifdef _MSC_VER
#ifdef _DEBUG
#define KUKU_DEBUG
#else
#undef KUKU_DEBUG
#endif
#else
#include "kuku/internal/config.h"
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <stdexcept>
#include <utility>

namespace kuku
{
    /**
    The type that represents a 128-bit item that can be added to the hash table.
    */
    using item_type = std::array<unsigned char, 16>;

    /**
    The type that represents a location in the hash table.
    */
    using location_type = std::uint32_t;

    /**
    The type that represents the size of a hash table.
    */
    using table_size_type = location_type;

    /**
    The smallest allowed table size.
    */
    constexpr table_size_type min_table_size = static_cast<table_size_type>(1);

    /**
    The largest allowed table size.
    */
    constexpr table_size_type max_table_size = static_cast<table_size_type>(1) << 30U;

    /**
    The smallest allowed number of hash functions.
    */
    constexpr std::uint32_t min_loc_func_count = 1;

    /**
    The largest allowed number of hash functions. This must be a power of two: QueryResult::found()
    masks loc_func_index against (max_loc_func_count - 1) to detect the "not found" sentinel
    (max_loc_func_count itself), which is only correct when max_loc_func_count is a power of two.
    */
    constexpr std::uint32_t max_loc_func_count = 32;
    static_assert(
        (max_loc_func_count & (max_loc_func_count - 1)) == 0,
        "max_loc_func_count must be a power of two for QueryResult::found() to work");

    constexpr int bytes_per_uint64 = sizeof(std::uint64_t);

    constexpr int bytes_per_item = sizeof(item_type);

    /**
    Generates a random 64-bit unsigned integer.
    */
    inline std::uint64_t random_uint64()
    {
        std::random_device rd;
        return (static_cast<std::uint64_t>(rd()) << 32U) | static_cast<std::uint64_t>(rd());
    }

    /**
    Returns the low 64-bit word of the item.

    Uses std::memcpy to avoid strict-aliasing/alignment UB: item_type is
    std::array<unsigned char, 16> with alignment 1, so reinterpret_cast<uint64_t*>
    on item.data() can be misaligned on strict-alignment targets (e.g., some ARM).
    */
    inline std::uint64_t get_low_word(const item_type &item) noexcept
    {
        std::uint64_t value{};
        std::memcpy(&value, item.data(), sizeof(value));
        return value;
    }

    /**
    Returns the high 64-bit word of the item. See get_low_word for the rationale
    behind the std::memcpy-based implementation.
    */
    inline std::uint64_t get_high_word(const item_type &item) noexcept
    {
        std::uint64_t value{};
        std::memcpy(&value, item.data() + bytes_per_uint64, sizeof(value));
        return value;
    }

    /**
    Stores a 64-bit value into the low word of the item.
    */
    inline void set_low_word(std::uint64_t value, item_type &destination) noexcept
    {
        std::memcpy(destination.data(), &value, sizeof(value));
    }

    /**
    Stores a 64-bit value into the high word of the item.
    */
    inline void set_high_word(std::uint64_t value, item_type &destination) noexcept
    {
        std::memcpy(destination.data() + bytes_per_uint64, &value, sizeof(value));
    }

    /**
    Sets the value of a given hash table item from a given buffer.

    @param[in] in The buffer to set the value from
    @param[out] destination The hash table item whose value is to be set
    */
    inline void set_item(const unsigned char *in, item_type &destination) noexcept
    {
        std::memcpy(destination.data(), in, bytes_per_item);
    }

    /**
    Creates a new hash table item and sets its value from a given buffer.

    @param[in] in The buffer to set the value from
    */
    inline item_type make_item(const unsigned char *in) noexcept
    {
        item_type out;
        set_item(in, out);
        return out;
    }

    /**
    Sets the value of a given hash table item from a pair of two 64-bit words.

    @param[in] low_word The low 64 bits of the value of the item
    @param[in] high_word The high 64 bits of the value of the item
    @param[out] destination The hash table item whose value is to be set
    */
    inline void set_item(std::uint64_t low_word, std::uint64_t high_word, item_type &destination) noexcept
    {
        set_low_word(low_word, destination);
        set_high_word(high_word, destination);
    }

    /**
    Creates a hash table item and sets its value from a pair of two 64-bit words.

    @param[in] low_word The lowest 64 bits of the value of the item
    @param[in] high_word The highest 64 bits of the value of the item
    */
    inline item_type make_item(std::uint64_t low_word, std::uint64_t high_word) noexcept
    {
        item_type item;
        set_item(low_word, high_word, item);
        return item;
    }

    /**
    Creates a zero hash table item.
    */
    inline item_type make_zero_item() noexcept
    {
        return item_type{};
    }

    /**
    Sets a given hash table item to zero.

    @param[out] destination The hash table item whose value is to be set
    */
    inline void set_zero_item(item_type &destination) noexcept
    {
        destination = item_type{};
    }

    /**
    Sets the value of a given hash table item to all one-bits.

    @param[out] destination The hash table item whose value is to be set
    */
    inline void set_all_ones_item(item_type &destination) noexcept
    {
        destination.fill(static_cast<unsigned char>(0xFF));
    }

    /**
    Creates a hash table item and sets its value to all one-bits.
    */
    inline item_type make_all_ones_item() noexcept
    {
        item_type item;
        set_all_ones_item(item);
        return item;
    }

    /**
    Returns whether a given hash table item is zero.

    @param[in] in The hash table item to test
    */
    inline bool is_zero_item(const item_type &in) noexcept
    {
        return !(get_low_word(in) | get_high_word(in));
    }

    /**
    Returns whether a given hash table item has all one-bits.

    @param[in] in The hash table item to test
    */
    inline bool is_all_ones_item(const item_type &in) noexcept
    {
        return !(~get_low_word(in) | ~get_high_word(in));
    }

    /**
    Returns whether two hash table items are equal.

    @param[in] in1 The first hash table item
    @param[in] in2 The second hash table item
    */
    inline bool are_equal_item(const item_type &in1, const item_type &in2) noexcept
    {
        return (get_low_word(in1) == get_low_word(in2)) && (get_high_word(in1) == get_high_word(in2));
    }

    /**
    Sets a given hash table item to a random value.

    @param[out] destination The hash table item whose value is to be set
    */
    inline void set_random_item(item_type &destination)
    {
        set_item(random_uint64(), random_uint64(), destination);
    }

    /**
    Creates a random hash table item.

    @throws std::exception derived if the underlying std::random_device fails (rare; possible on
    locked-down systems where /dev/urandom is unavailable, etc.).
    */
    inline item_type make_random_item()
    {
        item_type out;
        set_random_item(out);
        return out;
    }

    /**
    Interprets a hash table item as a 128-bit integer and increments its value by one.

    @param[in,out] in The hash table item whose value is to be incremented
    */
    inline void increment_item(item_type &in) noexcept
    {
        std::uint64_t low_word = get_low_word(in) + 1;
        std::uint64_t high_word = get_high_word(in) + (low_word == 0 ? 1 : 0);
        set_low_word(low_word, in);
        set_high_word(high_word, in);
    }
} // namespace kuku
