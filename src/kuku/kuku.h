// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/locfunc.h"
#include <vector>
#include <random>
#include <memory>
#include <stdexcept>
#include <set>

namespace kuku
{
    class QueryResult
    {
        friend class KukuTable;

    public:
        QueryResult() = default;

        inline location_type location() const noexcept
        {
            return location_;
        }

        inline std::size_t loc_func_index() const noexcept
        {
            return loc_func_index_;
        }

        inline bool in_stash() const noexcept
        {
            return !~loc_func_index_;
        }

        inline operator bool() const
        {
            return !(loc_func_index_ & ~(max_loc_func_count - 1)) || in_stash();
        }

    private:
        QueryResult(location_type location, std::size_t loc_func_index)
            : location_(location), loc_func_index_(loc_func_index)
        {
#ifdef SEAL_DEBUG
            if (location >= max_table_size)
            {
                throw std::invalid_argument("invalid location");
            }
#endif
        }

        location_type location_ = 0;

        std::size_t loc_func_index_ = 0;
    };

    class KukuTable
    {
    public:
        /*
        Creates a new kuku hash table.
        */
        KukuTable(
            int log_table_size,
            table_size_type stash_size,
            std::size_t loc_func_count,
            item_type loc_func_seed,
            std::uint64_t max_probe,
            item_type empty_item);

        /*
        Adds a single item to the KukuTable using random-walk kuku hashing.
        */
        inline bool insert(item_type item)
        {
            // Return false if the item already exists in the table
            if (query(item))
            {
                return false;
            }
            return insert(item, 0);
        }

        /*
        Returns true of the provided item is contained in the hash table.
        */
        QueryResult query(item_type item) const;

        /*
        Returns the locations that this item may live at.
        */
        inline location_type location(
            item_type item,
            std::size_t loc_func_index) const
        {
            return loc_funcs_[loc_func_index](item);
        }

        /*
        Returns the locations that this item may live at.
        */
        std::set<location_type> all_locations(item_type item) const;

        /*
        Clears the table by filling every location with the empty item.
        */
        void clear_table();

        inline std::size_t loc_func_count() const noexcept
        {
            return loc_funcs_.size();
        }

        inline const std::vector<item_type> &table() const noexcept
        {
            return table_;
        }

        inline const item_type &table(location_type index) const
        {
            return table_[index];
        }

        inline const std::vector<item_type> &stash() const noexcept
        {
            return stash_;
        }

        inline const item_type &stash(location_type index) const
        {
            if (index >= stash_.size() && index < stash_size_)
            {
                return empty_item_;
            }
            return stash_[index];
        }

        inline int log_table_size() const noexcept
        {
            return log_table_size_;
        }

        inline table_size_type table_size() const noexcept
        {
            return table_size_type(1) << log_table_size();
        }

        inline table_size_type stash_size() const noexcept
        {
            return stash_size_;
        }

        inline item_type loc_func_seed() const noexcept
        {
            return loc_func_seed_;
        }

        inline std::uint64_t max_probe() const noexcept
        {
            return max_probe_;
        }

        inline const item_type &empty_item() const noexcept
        {
            return empty_item_;
        }

        /*
        Returns whether a given location in the table is empty, i.e., contains
        the empty item.
        */
        inline bool is_empty(location_type index) const noexcept
        {
            return is_empty_item(table_[index]);
        }

        /*
        Returns whether a given item is the empty item.
        */
        inline bool is_empty_item(const item_type &item) const noexcept
        {
            return are_equal_item(item, empty_item_);
        }

        inline item_type last_insert_fail_item() const noexcept
        {
            return last_insert_fail_item_;
        }

        inline double fill_rate() const noexcept
        {
            return static_cast<double>(inserted_items_) /
                static_cast<double>(table_size() + stash_size_);
        }

    private:
        KukuTable(const KukuTable &copy) = delete;

        KukuTable &operator =(const KukuTable &assign) = delete;

        void generate_loc_funcs(std::size_t loc_func_count, item_type seed);

        /*
        Insertion of an element using random-walk kuku hashing.
        */
        bool insert(item_type item, std::uint64_t level);

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
        log2 of the number of slots in the table.
        */
        int log_table_size_;

        /*
        The size of the stash.
        */
        table_size_type stash_size_;

        /*
        Seed for the hash functions
        */
        item_type loc_func_seed_;

        /*
        The maximum number of attempts that are made to insert an item.
        */
        std::uint64_t max_probe_;

        /*
        An item value that denotes an empty item.
        */
        item_type empty_item_;

        /*
        Storage for an item that was evicted and could not be re-inserted. This
        is populated when insert fails.
        */
        item_type last_insert_fail_item_;

        /*
        The number of items that have been inserted to table or stash.
        */
        table_size_type inserted_items_;

        /*
        Randomness source.
        */
        std::random_device rd_;
    };
}
