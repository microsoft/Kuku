// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

namespace kuku
{
    /**
    The type that represents a 128-bit item that can be added to the hash table.
    */
    using item_type = std::array<unsigned char, 16>;

    // Kuku's table layout is defined in terms of LITTLE-ENDIAN byte order, independently of
    // the target's native byte order.
    //
    // An item is 16 raw bytes, and those raw bytes are what BLAKE2xb hashes. Two places
    // interpret bytes as integers: the word accessors below (on which increment_item is
    // built), and HashFunc, which lets BLAKE2xb write its output directly into an array of
    // location_type. Both read and write those integers in little-endian byte order, so the
    // derived location functions - and hence the table layout - are the same on every
    // target. This matters because Kuku's consumers are two-party protocols that must agree
    // on where items live, and the two parties need not share a byte order.
    //
    // On a little-endian target the conversions are the identity, and the accessors reduce
    // to a single std::memcpy; that fast path is selected here. Every other target - a
    // big-endian one, or a compiler that exposes no byte-order macro - takes a portable
    // byte-by-byte path that is correct whatever the native byte order is. Correctness
    // therefore never depends on this detection being right; only speed does.
    //
    // MSVC defines no __BYTE_ORDER__, but every architecture it targets - x86, x64, ARM,
    // ARM64 - is little-endian.
#if defined(_MSC_VER) || \
    (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
#define KUKU_NATIVE_LITTLE_ENDIAN
#endif

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
    The smallest allowed stash size. A stash is optional, so zero is valid.
    */
    constexpr table_size_type min_stash_size = static_cast<table_size_type>(0);

    /**
    The largest allowed stash size.

    A query that does not find an item in the table scans the entire stash linearly, so an
    unbounded stash degrades the table into a linear search and, because the stash grows one
    item at a time up to stash_size, an unbounded value is also a memory-exhaustion hazard.

    The bound is set from the measured point at which a stash scan costs as much as probing the
    table. In this implementation one stash comparison costs roughly 0.6 ns while probing the
    table costs roughly 28 ns for three location functions over a small table and roughly 60-67
    ns over a large one, putting the crossover between about 45 and 110 entries for the common
    configuration. 128 is the next power of two above that crossover. It is also more than an
    order of magnitude larger than any stash size seen in practice, and it bounds the stash at
    128 * 16 = 2 KiB, small enough to reserve outright.
    */
    constexpr table_size_type max_stash_size = static_cast<table_size_type>(1) << 7U;

    static_assert(
        max_stash_size <= max_table_size,
        "max_stash_size must not exceed max_table_size, so that a stash index returned in a "
        "QueryResult is always within the range of a table location");

    static_assert(
        static_cast<std::uint64_t>(max_table_size) + static_cast<std::uint64_t>(max_stash_size) <=
            static_cast<std::uint64_t>((std::numeric_limits<table_size_type>::max)()),
        "max_table_size + max_stash_size must fit in table_size_type so that the inserted item "
        "counter cannot wrap");

    /**
    The smallest allowed number of hash functions.
    */
    constexpr std::uint32_t min_loc_func_count = 1;

    /**
    The largest allowed number of hash functions.

    This must be a power of two, for the following reason. QueryResult packs three different things
    into one std::uint32_t field, loc_func_index_:

        0 .. max_loc_func_count - 1   the index of the location function that found the item
        max_loc_func_count            the "not found in the table or the stash" sentinel
        ~std::uint32_t(0)             the "found in the stash" sentinel

    QueryResult::found() has to separate the first row from the second, which it does by testing
    loc_func_index_ against ~(max_loc_func_count - 1), the mask of every bit above the largest
    valid index. That mask is only the correct discriminator when max_loc_func_count is a power of
    two, hence the static_assert below. The third row fails that test as well, which is why found()
    ORs in in_stash() to rescue it. No fourth value can occur: the two-argument QueryResult
    constructor is private and KukuTable is its only friend, so the only other way to obtain a
    QueryResult is the public default constructor, which initializes loc_func_index_ to the
    not-found sentinel of the second row.
    */
    constexpr std::uint32_t max_loc_func_count = 32;
    static_assert(
        (max_loc_func_count & (max_loc_func_count - 1)) == 0,
        "max_loc_func_count must be a power of two for QueryResult::found() to work");
    static_assert(
        max_loc_func_count != ~static_cast<std::uint32_t>(0), "the not-found and in-stash sentinels must be distinct");

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
    Reads a 64-bit word stored in little-endian byte order. See the note on byte order above.

    On a little-endian target this is a std::memcpy, which avoids strict-aliasing/alignment
    UB: item_type is std::array<unsigned char, 16> with alignment 1, so reinterpret_cast<uint64_t*>
    on item.data() can be misaligned on strict-alignment targets (e.g., some ARM).
    */
    inline std::uint64_t load_word(const unsigned char *in) noexcept
    {
#if defined(KUKU_NATIVE_LITTLE_ENDIAN)
        std::uint64_t value{};
        std::memcpy(&value, in, sizeof(value));
        return value;
#else
        std::uint64_t value = 0;
        for (std::size_t i = 0; i < sizeof(value); i++)
        {
            value |= static_cast<std::uint64_t>(in[i]) << (8U * i);
        }
        return value;
#endif
    }

    /**
    Stores a 64-bit word in little-endian byte order. See load_word.
    */
    inline void store_word(std::uint64_t value, unsigned char *out) noexcept
    {
#if defined(KUKU_NATIVE_LITTLE_ENDIAN)
        std::memcpy(out, &value, sizeof(value));
#else
        for (std::size_t i = 0; i < sizeof(value); i++)
        {
            out[i] = static_cast<unsigned char>(value >> (8U * i));
        }
#endif
    }

    /**
    Returns the low 64-bit word of the item.
    */
    inline std::uint64_t get_low_word(const item_type &item) noexcept
    {
        return load_word(item.data());
    }

    /**
    Returns the high 64-bit word of the item.
    */
    inline std::uint64_t get_high_word(const item_type &item) noexcept
    {
        return load_word(item.data() + bytes_per_uint64);
    }

    /**
    Stores a 64-bit value into the low word of the item.
    */
    inline void set_low_word(std::uint64_t value, item_type &destination) noexcept
    {
        store_word(value, destination.data());
    }

    /**
    Stores a 64-bit value into the high word of the item.
    */
    inline void set_high_word(std::uint64_t value, item_type &destination) noexcept
    {
        store_word(value, destination.data() + bytes_per_uint64);
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
