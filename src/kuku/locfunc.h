// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/internal/hash.h"
#include <stdexcept>

namespace kuku
{
    /**
    An instance of the LocFunc class represents a location function (hash function) used by the KukuTable class to
    insert an item in the hash table. The location functions are automatically created by the KukuTable class instance
    from a seed.
    */
    class LocFunc
    {
    public:
        /**
        Creates a new location function for a table of given size. The location function uses the given seed for
        randomness.

        @param[in] table_size The size of the hash table that this location function is for
        @param[in] seed The seed for randomness
        @throws std::invalid_argument if the table_size is larger or smaller than allowed
        */
        LocFunc(table_size_type table_size, item_type seed) : table_size_(table_size), hf_(seed)
        {
            if (table_size < min_table_size || table_size > max_table_size)
            {
                throw std::invalid_argument("table_size is out of range");
            }
        }

        /**
        Creates a copy of a given location function.

        @param[in] copy The location function to copy from
        */
        LocFunc(const LocFunc &copy) = default;

        /**
        Assigns this location function to be equal to a given location function.

        @param[in] assign The location function to assign from
        */
        LocFunc &operator=(const LocFunc &assign) = delete;

        /**
        Returns the location for a given item.

        @param[in] item The hash table item for which to compute the location
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
