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
    constexpr table_size_type min_table_size = table_size_type(1);

    /**
    The largest allowed table size.
    */
    constexpr table_size_type max_table_size = table_size_type(1) << 30;

    /**
    The smallest allowed number of hash functions.
    */
    constexpr std::uint32_t min_loc_func_count = 1;

    /**
    The largest allowed number of hash functions. This must be a power of two for correct behavior.
    */
    constexpr std::uint32_t max_loc_func_count = 32;

    constexpr int bytes_per_uint64 = sizeof(std::uint64_t);

    constexpr int bytes_per_item = sizeof(item_type);

    /**
    Generates a random 64-bit unsigned integer.
    */
    inline std::uint64_t random_uint64()
    {
        std::random_device rd;
        return (static_cast<std::uint64_t>(rd()) << 32) | static_cast<std::uint64_t>(rd());
    }

    /**
    Return a reference to the low-word of the item.
    */
    inline std::uint64_t &get_low_word(item_type &item)
    {
        return *reinterpret_cast<std::uint64_t*>(item.data());
    }

    /**
    Return a reference to the high-word of the item.
    */
    inline std::uint64_t &get_high_word(item_type &item)
    {
        return *reinterpret_cast<std::uint64_t*>(item.data() + 8);
    }

    /**
    Return a reference to the low-word of the item.
    */
    inline std::uint64_t get_low_word(const item_type &item)
    {
        return *reinterpret_cast<const std::uint64_t*>(item.data());
    }

    /**
    Return a reference to the high-word of the item.
    */
    inline std::uint64_t get_high_word(const item_type &item)
    {
        return *reinterpret_cast<const std::uint64_t*>(item.data() + 8);
    }

    /**
    Sets the value of a given hash table item from a given buffer.

    @param[in] in The buffer to set the value from
    @param[out] destination The hash table item whose value is to be set
    */
    inline void set_item(const unsigned char *in, item_type &destination) noexcept
    {
        std::copy_n(in, bytes_per_item, destination.data());
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
        get_low_word(destination) = low_word;
        get_high_word(destination) = high_word;
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
        get_low_word(destination) = ~std::uint64_t(0);
        get_high_word(destination) = ~std::uint64_t(0);
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
    Returns whether a given has table item has all one-bits.

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
    inline void set_random_item(item_type &destination) noexcept
    {
        set_item(random_uint64(), random_uint64(), destination);
    }

    /**
    Creates a random hash table item.
    */
    inline item_type make_random_item() noexcept
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
        get_low_word(in) += 1;
        get_high_word(in) += !get_low_word(in) ? 1 : 0;
    }
} // namespace kuku
