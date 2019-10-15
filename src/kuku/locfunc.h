// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4804)
#endif
#include "kuku/blake2.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <cstring>
#include <stdexcept>

namespace kuku
{
    class LocFunc
    {
    public:
        LocFunc(int log_table_size, item_type seed) : seed_(seed)
        {
            if (log_table_size < 1 || log_table_size > max_log_table_size)
            {
                throw std::invalid_argument("invalid log_table_size");
            }

            table_size_ = std::size_t(1) << log_table_size;
        }

        LocFunc(const LocFunc &copy) = default;

        LocFunc &operator =(const LocFunc &assign) = delete;

        /*
        Returns an integer between 0 and (table_size-1).
        */
        inline std::size_t operator ()(item_type item) const
        {
            return compress(item) & (table_size_ - 1);
        }

    private:
        inline std::size_t compress(item_type item) const
        {
            std::size_t out;
            blake2b(
                reinterpret_cast<unsigned char*>(&out),
                sizeof(out),
                reinterpret_cast<const unsigned char*>(item.data()),
                sizeof(item),
                reinterpret_cast<const unsigned char*>(seed_.data()), sizeof(seed_));
            return out;
        }

        std::size_t table_size_;

        item_type seed_;
    };
}
