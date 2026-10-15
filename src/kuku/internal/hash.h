// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/internal/blake2.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace kuku
{
    class HashFunc
    {
    public:
        HashFunc(item_type seed)
        {
            if (blake2xb(
                    random_array_.data(), random_array_size_ * sizeof(location_type), seed.data(), sizeof(seed),
                    nullptr, 0) != 0)
            {
                throw std::runtime_error("blake2xb failed");
            }
#if !defined(KUKU_NATIVE_LITTLE_ENDIAN)
            // BLAKE2xb wrote its output into random_array_ as raw bytes. Reread each entry as a
            // little-endian word, so that the tabulation table - and hence every location this
            // function returns - matches a little-endian peer's. See the note on byte order in
            // kuku/common.h.
            for (location_type &entry : random_array_)
            {
                std::array<unsigned char, sizeof(location_type)> bytes{};
                std::memcpy(bytes.data(), &entry, bytes.size());
                entry = 0;
                for (std::size_t i = 0; i < bytes.size(); i++)
                {
                    entry |= static_cast<location_type>(bytes[i]) << (8U * i);
                }
            }
#endif
        }

        location_type operator()(item_type item) const noexcept
        {
            // clang-format off
            // Deliberately one entry per input byte. This is the tabulation lookup, and the
            // one-per-line alignment is what makes the 16 blocks and their indices checkable
            // at a glance; letting the formatter pack them to the column limit would obscure
            // exactly the thing a reader needs to verify.
            return
                random_array_[(0 * block_value_count_) + static_cast<std::size_t>(item[0])] ^
                random_array_[(1 * block_value_count_) + static_cast<std::size_t>(item[1])] ^
                random_array_[(2 * block_value_count_) + static_cast<std::size_t>(item[2])] ^
                random_array_[(3 * block_value_count_) + static_cast<std::size_t>(item[3])] ^
                random_array_[(4 * block_value_count_) + static_cast<std::size_t>(item[4])] ^
                random_array_[(5 * block_value_count_) + static_cast<std::size_t>(item[5])] ^
                random_array_[(6 * block_value_count_) + static_cast<std::size_t>(item[6])] ^
                random_array_[(7 * block_value_count_) + static_cast<std::size_t>(item[7])] ^
                random_array_[(8 * block_value_count_) + static_cast<std::size_t>(item[8])] ^
                random_array_[(9 * block_value_count_) + static_cast<std::size_t>(item[9])] ^
                random_array_[(10 * block_value_count_) + static_cast<std::size_t>(item[10])] ^
                random_array_[(11 * block_value_count_) + static_cast<std::size_t>(item[11])] ^
                random_array_[(12 * block_value_count_) + static_cast<std::size_t>(item[12])] ^
                random_array_[(13 * block_value_count_) + static_cast<std::size_t>(item[13])] ^
                random_array_[(14 * block_value_count_) + static_cast<std::size_t>(item[14])] ^
                random_array_[(15 * block_value_count_) + static_cast<std::size_t>(item[15])];
            // clang-format on
        }

    private:
        static constexpr std::size_t block_size_ = 1;

        static constexpr std::size_t block_count_ = sizeof(item_type);

        static constexpr std::size_t block_value_count_ = (static_cast<std::size_t>(1) << (8 * block_size_));

        static constexpr std::size_t random_array_size_ = block_value_count_ * block_count_;

        std::array<location_type, random_array_size_> random_array_{};
    };
} // namespace kuku
