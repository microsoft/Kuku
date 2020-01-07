// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/hash.h"
#include <cstring>
#include <stdexcept>

namespace kuku
{
    class LocFunc
    {
    public:
        LocFunc(int log_table_size, item_type seed) : hf_(seed)
        {
            if (log_table_size < 1 || log_table_size > max_log_table_size)
            {
                throw std::invalid_argument("invalid log_table_size");
            }

            table_size_ = table_size_type(1) << log_table_size;
        }

        LocFunc(const LocFunc &copy) = default;

        LocFunc &operator =(const LocFunc &assign) = delete;

        /*
        Returns an integer between 0 and (table_size - 1).
        */
        inline location_type operator ()(item_type item) const noexcept
        {
            return hf_(item) & (table_size_ - 1);
        }

    private:
        table_size_type table_size_;

        HashFunc hf_;
    };
}
