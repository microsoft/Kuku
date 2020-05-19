// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/internal/hash.h"
#include <stdexcept>

namespace kuku
{
    class LocFunc
    {
    public:
        LocFunc(table_size_type table_size, item_type seed) : table_size_(table_size), hf_(seed)
        {
            if (table_size < min_table_size || table_size > max_table_size)
            {
                throw std::invalid_argument("table_size is out of range");
            }
        }

        LocFunc(const LocFunc &copy) = default;

        LocFunc &operator=(const LocFunc &assign) = delete;

        /*
        Returns an integer between 0 and (table_size - 1).
        */
        inline location_type operator()(item_type item) const noexcept
        {
            return hf_(item) % table_size_;
        }

    private:
        table_size_type table_size_;

        HashFunc hf_;
    };
} // namespace kuku
