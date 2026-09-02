// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "kuku/kuku.h"

using namespace std;

namespace kuku
{
    QueryResult KukuTable::query(item_type item) const
    {
        if (is_empty_item(item))
        {
            throw invalid_argument("item cannot be the empty item");
        }

        // Search the hash table
        for (uint32_t i = 0; i < loc_func_count(); i++)
        {
            auto loc = location(item, i);
            if (are_equal_item(table_[loc], item))
            {
                return { loc, i };
            }
        }

        // Search the stash
        for (location_type loc = 0; loc < stash_.size(); loc++)
        {
            if (are_equal_item(stash_[loc], item))
            {
                return { loc, ~static_cast<uint32_t>(0) };
            }
        }

        // Not found
        return { 0, max_loc_func_count };
    }

    KukuTable::KukuTable(
        table_size_type table_size, table_size_type stash_size, uint32_t loc_func_count, item_type loc_func_seed,
        uint64_t max_probe, item_type empty_item)
        : table_size_(table_size), stash_size_(stash_size), loc_func_seed_(loc_func_seed), max_probe_(max_probe),
          empty_item_(empty_item), leftover_item_(empty_item_), gen_(random_uint64())
    {
        if (loc_func_count < min_loc_func_count || loc_func_count > max_loc_func_count)
        {
            throw invalid_argument("loc_func_count is out of range");
        }
        if (table_size < min_table_size || table_size > max_table_size)
        {
            throw invalid_argument("table_size is out of range");
        }
        // min_stash_size is zero, so only the upper bound needs testing; comparing an unsigned
        // value against zero would be tautological.
        if (stash_size > max_stash_size)
        {
            throw invalid_argument("stash_size is out of range");
        }
        if (!max_probe)
        {
            throw invalid_argument("max_probe cannot be zero");
        }

        // Allocate the hash table
        table_.resize(table_size_, empty_item_);

        // The stash is bounded by max_stash_size, so reserving it outright costs at most
        // max_stash_size * sizeof(item_type) = 2 KiB and removes any reallocation during insert.
        stash_.reserve(stash_size_);

        // Create the location (hash) functions
        generate_loc_funcs(loc_func_count, loc_func_seed_);

        // Set up the distribution for location function sampling
        u_ = std::uniform_int_distribution<uint32_t>(0, loc_func_count - 1);
    }

    set<location_type> KukuTable::all_locations(item_type item) const
    {
        if (is_empty_item(item))
        {
            throw invalid_argument("item cannot be the empty item");
        }

        set<location_type> result;
        for (auto lf : loc_funcs_)
        {
            result.emplace(lf(item));
        }
        return result;
    }

    void KukuTable::clear_table() noexcept
    {
        std::fill(table_.begin(), table_.end(), empty_item_);
        stash_.clear();
        leftover_item_ = empty_item_;
        inserted_items_ = 0;
    }

    void KukuTable::generate_loc_funcs(uint32_t loc_func_count, item_type seed)
    {
        loc_funcs_.clear();
        while (loc_func_count--)
        {
            loc_funcs_.emplace_back(table_size_, seed);
            increment_item(seed);
        }
    }

    bool KukuTable::insert(item_type item)
    {
        // Record every write the random walk makes, so that a walk which ends in failure can be
        // undone. This is done before anything else, including the duplicate check below, so that
        // no earlier call's journal can ever survive into this one. clear() retains the capacity
        // allocated by earlier calls.
        eviction_journal_.clear();

        // Check if the item is already inserted or is the empty item
        if (query(item))
        {
            return false;
        }

        try
        {
            uint64_t level = max_probe_;
            while (level--)
            {
                // Loop over all possible locations
                for (uint32_t i = 0; i < loc_func_count(); i++)
                {
                    location_type loc = location(item, i);
                    if (is_empty_item(table_[loc]))
                    {
                        table_[loc] = item;
                        inserted_items_++;
                        return true;
                    }
                }

                // Swap in the current item and in next round try the popped out item. The journal
                // entry is appended BEFORE the write: push_back offers the strong guarantee, so if
                // it throws, the journal still describes exactly the writes that were applied.
                location_type loc = location(item, static_cast<uint32_t>(u_(gen_)));
                eviction_journal_.push_back(loc);
                item = swap(item, loc);
            }

            // level reached zero; try stash
            if (stash_.size() < stash_size_)
            {
                stash_.push_back(item);
                inserted_items_++;
                return true;
            }
        }
        catch (...)
        {
            // An allocation failure part-way through the walk must not leave the table in a
            // half-evicted state either.
            (void)unwind_eviction_chain(item);
            throw;
        }

        // Neither the table nor the stash could take the item at the head of the chain. Undo the
        // whole chain so that this call has no observable effect on the table, the stash or
        // inserted_items_. The unwind hands back the head of the chain, which by construction is
        // exactly the item this function was called with.
        leftover_item_ = unwind_eviction_chain(item);
        return false;
    }
} // namespace kuku
