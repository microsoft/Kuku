// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4804)
#endif
#include "kuku/internal/blake2.h"
#ifdef _MSC_VER
#   pragma warning(pop)
#endif
#include <array>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <cstddef>

namespace kuku
{
    class HashFunc
    {
    public:
        HashFunc(item_type seed)
        {
            if (blake2xb(
                random_array_.data(),
                random_array_size_ * sizeof(location_type),
                seed.data(),
                sizeof(seed),
                nullptr, 0) != 0)
            {
                throw std::runtime_error("blake2xb failed");
            }
        }

        inline location_type operator ()(item_type item) const noexcept
        {
            return
                random_array_[0 * block_value_count_ + static_cast<std::size_t>(item[0] & 0x00000000000000FFULL)] ^
                random_array_[1 * block_value_count_ + static_cast<std::size_t>((item[0] & 0x000000000000FF00ULL) >> 8)] ^
                random_array_[2 * block_value_count_ + static_cast<std::size_t>((item[0] & 0x0000000000FF0000ULL) >> 16)] ^
                random_array_[3 * block_value_count_ + static_cast<std::size_t>((item[0] & 0x00000000FF000000ULL) >> 24)] ^
                random_array_[4 * block_value_count_ + static_cast<std::size_t>((item[0] & 0x000000FF00000000ULL) >> 32)] ^
                random_array_[5 * block_value_count_ + static_cast<std::size_t>((item[0] & 0x0000FF0000000000ULL) >> 40)] ^
                random_array_[6 * block_value_count_ + static_cast<std::size_t>((item[0] & 0x00FF000000000000ULL) >> 48)] ^
                random_array_[7 * block_value_count_ + static_cast<std::size_t>((item[0] & 0xFF00000000000000ULL) >> 56)] ^
                random_array_[8 * block_value_count_ + static_cast<std::size_t>(item[1] & 0x00000000000000FFULL)] ^
                random_array_[9 * block_value_count_ + static_cast<std::size_t>((item[1] & 0x000000000000FF00ULL) >> 8)] ^
                random_array_[10 * block_value_count_ + static_cast<std::size_t>((item[1] & 0x0000000000FF0000ULL) >> 16)] ^
                random_array_[11 * block_value_count_ + static_cast<std::size_t>((item[1] & 0x00000000FF000000ULL) >> 24)] ^
                random_array_[12 * block_value_count_ + static_cast<std::size_t>((item[1] & 0x000000FF00000000ULL) >> 32)] ^
                random_array_[13 * block_value_count_ + static_cast<std::size_t>((item[1] & 0x0000FF0000000000ULL) >> 40)] ^
                random_array_[14 * block_value_count_ + static_cast<std::size_t>((item[1] & 0x00FF000000000000ULL) >> 48)] ^
                random_array_[15 * block_value_count_ + static_cast<std::size_t>((item[1] & 0xFF00000000000000ULL) >> 56)];
        }

    private:
        static constexpr std::size_t block_size_ = 1;

        static constexpr std::size_t block_count_ = sizeof(item_type);

        static constexpr std::size_t block_value_count_ = (std::size_t(1) << (8 * block_size_));

        static constexpr std::size_t random_array_size_ = block_value_count_ * block_count_;

        static constexpr std::uint32_t block_mask_ =
            static_cast<std::uint32_t>(block_value_count_ - 1);

        std::array<location_type, random_array_size_> random_array_;
    };
}