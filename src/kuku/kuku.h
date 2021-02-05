// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/locfunc.h"
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace kuku
{
    class QueryResult;

    /**
    The KukuTable class represents a cockoo hash table. It includes information about the location functions (hash
    functions) and holds the items inserted into the table.
    */
    class KukuTable
    {
    public:
        /**
        Creates a new empty hash table.

        @param[in] table_size The size of the hash table
        @param[in] stash_size The size of the stash (possibly zero)
        @param[in] loc_func_count The number of location functions (hash functions) to use
        @param[in] loc_func_seed The 128-bit seed for the location functions, represented as a hash table item
        @param[in] max_probe The maximum number of random walk steps taken in attempting to insert an item
        @param[in] empty_item A hash table item that represents an empty location in the table
        @throws std::invalid_argument if loc_func_count is too large or too small
        @throws std::invalid_argument if table_size is too large or too small
        @throws std::invalid_argument if max_probe is zero
        */
        KukuTable(
            table_size_type table_size, table_size_type stash_size, std::uint32_t loc_func_count,
            item_type loc_func_seed, std::uint64_t max_probe, item_type empty_item);

        /**
        Adds a single item to the hash table using random walk cuckoo hashing. The return value indicates whether
        the item was successfully inserted (possibly into the stash) or not.

        @param[in] item The hash table item to insert
        @throws std::invalid_argument if the given item is the empty item for this hash table
        */
        bool insert(item_type item);

        /**
        Queries for the presence of a given item in the hash table and stash.

        @param[in] item The hash table item to query
        @throws std::invalid_argument if the given item is the empty item for this hash table
        */
        QueryResult query(item_type item) const;

        /**
        Returns a location that a given hash table item may be placed at.

        @param[in] item The hash table item for which the location is to be obtained
        @param[in] loc_func_index The index of the location function which to use to compute the location
        @throws std::out_of_range if loc_func_index is out of range
        @throws std::invalid_argument if the given item is the empty item for this hash table
        */
        inline location_type location(item_type item, std::uint32_t loc_func_index) const
        {
            if (loc_func_index >= loc_funcs_.size())
            {
                throw std::out_of_range("loc_func_index is out of range");
            }
            if (is_empty_item(item))
            {
                throw std::invalid_argument("item cannot be the empty item");
            }
            return loc_funcs_[loc_func_index](item);
        }

        /**
        Returns all hash table locations that this item may be placed at.

        @throws std::invalid_argument if the given item is the empty item for this hash table
        */
        std::set<location_type> all_locations(item_type item) const;

        /**
        Clears the hash table by filling every location with the empty item.
        */
        void clear_table();

        /**
        Returns the number of location functions used by the hash table.
        */
        inline std::uint32_t loc_func_count() const noexcept
        {
            return static_cast<std::uint32_t>(loc_funcs_.size());
        }

        /**
        Returns a reference to the hash table.
        */
        inline const std::vector<item_type> &table() const noexcept
        {
            return table_;
        }

        /**
        Returns a reference to a specific location in the hash table.

        @param[in] index The index in the hash table
        @throws std::out_of_range if index is out of range
        */
        inline const item_type &table(location_type index) const
        {
            if (index >= table_size_)
            {
                throw std::out_of_range("index is out of range");
            }
            return table_[index];
        }

        /**
        Returns a reference to the stash.
        */
        inline const std::vector<item_type> &stash() const noexcept
        {
            return stash_;
        }

        /**
        Returns a reference to a specific location in the stash.

        @param[in] index The index in the stash
        @throws std::out_of_range if index is out of range
        */
        inline const item_type &stash(location_type index) const
        {
            if (index >= stash_size_)
            {
                throw std::out_of_range("index is out of range");
            }
            if (index >= stash_.size() && index < stash_size_)
            {
                return empty_item_;
            }
            return stash_[index];
        }

        /**
        Returns the size of the hash table.
        */
        inline table_size_type table_size() const noexcept
        {
            return table_size_;
        }

        /**
        Returns the size of the stash.
        */
        inline table_size_type stash_size() const noexcept
        {
            return stash_size_;
        }

        /**
        Returns the 128-bit seed used for the location functions, represented as a hash table item.
        */
        inline item_type loc_func_seed() const noexcept
        {
            return loc_func_seed_;
        }

        /**
        Returns the maximum number of random walk steps taken in attempting to insert an item.
        */
        inline std::uint64_t max_probe() const noexcept
        {
            return max_probe_;
        }

        /**
        Returns the hash table item that represents an empty location in the table.
        */
        inline const item_type &empty_item() const noexcept
        {
            return empty_item_;
        }

        /**
        Returns whether a given location in the table is empty.

        @param[in] index The index in the hash table
        @throws std::out_of_range if index is out of range
        */
        inline bool is_empty(location_type index) const noexcept
        {
            return is_empty_item(table(index));
        }

        /**
        Returns whether a given item is the empty item for this hash table.

        @param[in] item The item to compare to the empty item
        */
        inline bool is_empty_item(const item_type &item) const noexcept
        {
            return are_equal_item(item, empty_item_);
        }

        /**
        When the insert function fails to insert a hash table item, there is a leftover item that could not be inserted
        into the table. This function will return the empty item if insertion never failed, and otherwise it will return
        the latest leftover item. Note that due to how the random walk insertion process works, the leftover item is
        usually not the same one that insert was called with.
        */
        inline item_type leftover_item() const noexcept
        {
            return leftover_item_;
        }

        /**
        Returns the current fill rate of the hash table and stash.
        */
        inline double fill_rate() const noexcept
        {
            return static_cast<double>(inserted_items_) /
                   (static_cast<double>(table_size()) + static_cast<double>(stash_size_));
        }

    private:
        KukuTable(const KukuTable &copy) = delete;

        KukuTable &operator=(const KukuTable &assign) = delete;

        void generate_loc_funcs(std::uint32_t loc_func_count, item_type seed);

        /*
        Swap an item in the table with a given item.
        */
        inline item_type swap(item_type item, location_type location) noexcept
        {
            item_type old_item = table_[location];
            table_[location] = item;
            return old_item;
        }

        /*
        The hash table that holds all of the input data.
        */
        std::vector<item_type> table_;

        /*
        The stash.
        */
        std::vector<item_type> stash_;

        /*
        The hash functions.
        */
        std::vector<LocFunc> loc_funcs_;

        /*
        The size of the table.
        */
        const table_size_type table_size_;

        /*
        The size of the stash.
        */
        const table_size_type stash_size_;

        /*
        Seed for the hash functions
        */
        const item_type loc_func_seed_;

        /*
        The maximum number of attempts that are made to insert an item.
        */
        const std::uint64_t max_probe_;

        /*
        An item value that denotes an empty item.
        */
        const item_type empty_item_;

        /*
        Storage for an item that was evicted and could not be re-inserted. This
        is populated when insert fails.
        */
        item_type leftover_item_;

        /*
        The number of items that have been inserted to table or stash.
        */
        table_size_type inserted_items_;

        /*
        Randomness source for location function sampling.
        */
        std::mt19937_64 gen_;

        std::uniform_int_distribution<std::uint32_t> u_;
    };

    /**
    The QueryResult class represents the result of a hash table query. It includes information about whether a queried
    item was found in the hash table, its location in the hash table or stash (if found), and the index of the location
    function (hash function) that was used to insert it. QueryResult objects are returned by the KukuTable::query
    function.
    */
    class QueryResult
    {
        friend class KukuTable;

    public:
        /**
        Creates a QueryResult object.
        */
        QueryResult() = default;

        /**
        Returns the hash table or stash location represented by this QueryResult.
        */
        inline location_type location() const noexcept
        {
            return location_;
        }

        /**
        Returns the index of the location function that was used to insert the queried item. This value is meaningless
        when in_stash() is true. A value equal to max_loc_func_count indicates the item was not found in the table or
        stash.
        */
        inline std::uint32_t loc_func_index() const noexcept
        {
            return loc_func_index_;
        }

        /**
        Returns whether the queried item was found in the stash.
        */
        inline bool in_stash() const noexcept
        {
            return !~loc_func_index_;
        }

        /**
        Returns whether the queried item was found in the hash table or in the stash.
        */
        inline bool found() const noexcept
        {
            return !(loc_func_index_ & ~(max_loc_func_count - 1)) || in_stash();
        }

        /**
        Returns whether the queried item was found in the hash table or in the stash.
        */
        inline operator bool() const noexcept
        {
            return found();
        }

    private:
        QueryResult(location_type location, std::uint32_t loc_func_index)
            : location_(location), loc_func_index_(loc_func_index)
        {
#ifdef KUKU_DEBUG
            if (location >= max_table_size)
            {
                throw std::invalid_argument("invalid location");
            }
#endif
        }

        location_type location_ = 0;

        std::uint32_t loc_func_index_ = 0;
    };
} // namespace kuku
