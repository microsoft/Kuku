// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// Include kuku/config.h only on platforms with CMake configuration (not MSVC).
#ifndef _MSC_VER
#include "kuku/config.h"
#endif

#include <cstddef>
#include <cstdint>
#include <utility>
#include <cstring>
#include <stdexcept>
#include <random>
#include <array>
#include <algorithm>

namespace kuku
{
    using item_type = std::array<std::uint64_t, 2>;

    using location_type = std::uint32_t;

    using table_size_type = location_type;

    constexpr int bytes_per_uint64 = sizeof(std::uint64_t);

    constexpr int bytes_per_item = sizeof(item_type);

    constexpr int max_log_table_size = 30;

    constexpr table_size_type max_table_size = table_size_type(1) << max_log_table_size;

    /*
    This must be a power of two.
    */
    constexpr std::size_t max_loc_func_count = 16;

    std::uint64_t random_uint64();

    inline void set_item(const unsigned char *in, item_type &destination) noexcept
    {
        std::copy_n(in, bytes_per_item, reinterpret_cast<unsigned char*>(destination.data()));
    }

    inline item_type make_item(const unsigned char *in) noexcept
    {
        item_type out;
        set_item(in, out);
        return out;
    }

    inline void set_item(
        std::uint64_t low_word,
        std::uint64_t high_word,
        item_type &destination) noexcept
    {
        destination[0] = low_word;
        destination[1] = high_word;
    }

    inline item_type make_item(
        std::uint64_t low_word, std::uint64_t high_word) noexcept
    {
        return { low_word, high_word };
    }

    inline item_type make_zero_item() noexcept
    {
        return make_item(0, 0);
    }

    inline void set_zero_item(item_type &destination) noexcept
    {
        destination[0] = 0;
        destination[1] = 0;
    }

    inline item_type make_all_ones_item() noexcept
    {
        return make_item(~std::uint64_t(0), ~std::uint64_t(0));
    }

    inline void set_all_ones_item(item_type &destination) noexcept
    {
        destination[0] = ~std::uint64_t(0);
        destination[1] = ~std::uint64_t(0);
    }

    inline bool is_zero_item(const item_type &in) noexcept
    {
        return !(in[0] | in[1]);
    }

    inline bool is_all_ones_item(const item_type &in) noexcept
    {
        return !(~in[0] | ~in[1]);
    }

    inline bool are_equal_item(const item_type &in1, const item_type &in2) noexcept
    {
        return (in1[0] == in2[0]) && (in1[1] == in2[1]);
    }

    inline void set_random_item(item_type &destination) noexcept
    {
        destination[0] = random_uint64();
        destination[1] = random_uint64();
    }

    inline item_type make_random_item() noexcept
    {
        item_type out;
        set_random_item(out);
        return out;
    }

    inline void increment_item(item_type &in) noexcept
    {
        in[0] += 1;
        in[1] += !in[0] ? 1 : 0;
    }
}
