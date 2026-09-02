// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/common.h"
#include "kuku/locfunc.h"
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace kuku
{
    class QueryResult;

    /**
    The KukuTable class represents a cuckoo hash table. It includes information about the location functions (hash
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
        @param[in] loc_func_seed The 128-bit seed from which all location functions are derived, represented as a
        hash table item. If an untrusted party can influence which items are inserted, sample this freshly for each
        table from a cryptographically secure random number generator and keep it from that party until its items
        are fixed; see https://github.com/microsoft/Kuku#security-model.
        @param[in] max_probe The maximum number of random walk steps taken in attempting to insert an item
        @param[in] empty_item A hash table item that represents an empty location in the table
        @throws std::invalid_argument if loc_func_count is too large or too small
        @throws std::invalid_argument if table_size is too large or too small
        @throws std::invalid_argument if stash_size is too large
        @throws std::invalid_argument if max_probe is zero
        */
        KukuTable(
            table_size_type table_size, table_size_type stash_size, std::uint32_t loc_func_count,
            item_type loc_func_seed, std::uint64_t max_probe, item_type empty_item);

        /**
        Adds a single item to the hash table using random walk cuckoo hashing. The return value indicates whether
        the item was successfully inserted (possibly into the stash) or not.

        This function is atomic. If it returns false, or if it exits by throwing, the hash table and the stash are
        left exactly as they were before the call: an item that cannot be inserted never displaces an item that was
        inserted earlier, and every item for which insert returned true stays findable with query until clear_table
        is called. When insert returns false because the random walk was exhausted and the stash was full,
        leftover_item() returns the item that was passed to this call; when it returns false because the item was
        already present in the table or the stash, leftover_item() is not modified.

        @param[in] item The hash table item to insert
        @throws std::invalid_argument if the given item is the empty item for this hash table
        @throws std::bad_alloc if the eviction journal cannot be grown; the hash table and the stash are left
        unmodified in this case as well. The stash is reserved to its full size when the table is constructed, so
        moving an item into it never allocates.
        */
        [[nodiscard]] bool insert(item_type item);

        /**
        Queries for the presence of a given item in the hash table and stash.

        @param[in] item The hash table item to query
        @throws std::invalid_argument if the given item is the empty item for this hash table
        */
        [[nodiscard]] QueryResult query(item_type item) const;

        /**
        Returns a location that a given hash table item may be placed at.

        @param[in] item The hash table item for which the location is to be obtained
        @param[in] loc_func_index The index of the location function which to use to compute the location
        @throws std::out_of_range if loc_func_index is out of range
        @throws std::invalid_argument if the given item is the empty item for this hash table
        */
        [[nodiscard]] location_type location(item_type item, std::uint32_t loc_func_index) const
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
        [[nodiscard]] std::set<location_type> all_locations(item_type item) const;

        /**
        Clears the hash table by filling every location with the empty item.
        */
        void clear_table() noexcept;

        /**
        Returns the number of location functions used by the hash table.
        */
        [[nodiscard]] std::uint32_t loc_func_count() const noexcept
        {
            return static_cast<std::uint32_t>(loc_funcs_.size());
        }

        /**
        Returns a reference to the hash table.
        */
        [[nodiscard]] const std::vector<item_type> &table() const noexcept
        {
            return table_;
        }

        /**
        Returns a reference to a specific location in the hash table.

        @param[in] index The index in the hash table
        @throws std::out_of_range if index is out of range
        */
        [[nodiscard]] const item_type &table(location_type index) const
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
        [[nodiscard]] const std::vector<item_type> &stash() const noexcept
        {
            return stash_;
        }

        /**
        Returns a reference to a specific location in the stash.

        For indices in the range [stash().size(), stash_size()), this returns a reference to the
        empty item rather than throwing — the stash is logically full of empty slots up to its
        configured maximum size, even before items have been pushed into it. Callers that want to
        distinguish "really stashed" from "padded empty" should iterate over stash() directly, or
        compare the return against empty_item().

        @param[in] index The index in the stash
        @throws std::out_of_range if index is greater than or equal to stash_size()
        */
        [[nodiscard]] const item_type &stash(location_type index) const
        {
            if (index >= stash_size_)
            {
                throw std::out_of_range("index is out of range");
            }
            if (index >= stash_.size())
            {
                return empty_item_;
            }
            return stash_[index];
        }

        /**
        Returns the size of the hash table.
        */
        [[nodiscard]] table_size_type table_size() const noexcept
        {
            return table_size_;
        }

        /**
        Returns the size of the stash.
        */
        [[nodiscard]] table_size_type stash_size() const noexcept
        {
            return stash_size_;
        }

        /**
        Returns the 128-bit seed used for the location functions, represented as a hash table item.
        */
        [[nodiscard]] item_type loc_func_seed() const noexcept
        {
            return loc_func_seed_;
        }

        /**
        Returns the maximum number of random walk steps taken in attempting to insert an item.
        */
        [[nodiscard]] std::uint64_t max_probe() const noexcept
        {
            return max_probe_;
        }

        /**
        Returns the hash table item that represents an empty location in the table.
        */
        [[nodiscard]] const item_type &empty_item() const noexcept
        {
            return empty_item_;
        }

        /**
        Returns whether a given location in the table is empty.

        @param[in] index The index in the hash table
        @throws std::out_of_range if index is out of range
        */
        [[nodiscard]] bool is_empty(location_type index) const
        {
            return is_empty_item(table(index));
        }

        /**
        Returns whether a given item is the empty item for this hash table.

        @param[in] item The item to compare to the empty item
        */
        [[nodiscard]] bool is_empty_item(const item_type &item) const noexcept
        {
            return are_equal_item(item, empty_item_);
        }

        /**
        Returns the item that the most recent failed call to insert could not place, or the empty item if no call to
        insert has failed since this hash table was created or last cleared.

        Because insert is atomic, this is always exactly the item that was passed to that call, which the caller
        already knows from the false return value of insert. This accessor is therefore redundant rather than
        dangerous: a failed insertion no longer leaves an earlier, unrelated item stranded here. It is retained for
        source compatibility and is expected to be deprecated and then removed in a future major version; prefer
        inspecting the return value of each insert call.

        Two properties of this value are easy to get wrong and will not change while the accessor exists. First, it
        is sticky: a later successful insert does not clear it, so a non-empty leftover item does not mean that the
        most recent insert failed. Second, insert also returns false when the item is already present in the table
        or the stash, and that case does NOT update the leftover item at all, so after a false return the leftover
        item may still belong to some earlier call. Only a single failure can ever be recovered this way; there is
        no way to enumerate several. Check the return value of every insert call instead of batching inserts and
        inspecting this afterwards.
        */
        [[nodiscard]] item_type leftover_item() const noexcept
        {
            return leftover_item_;
        }

        /**
        Returns the current fill rate of the hash table and stash.
        */
        [[nodiscard]] double fill_rate() const noexcept
        {
            return static_cast<double>(inserted_items_) /
                   (static_cast<double>(table_size()) + static_cast<double>(stash_size_));
        }

        KukuTable(const KukuTable &copy) = delete;

        KukuTable &operator=(const KukuTable &assign) = delete;

    private:
        void generate_loc_funcs(std::uint32_t loc_func_count, item_type seed);

        /*
        Swap an item in the table with a given item.
        */
        item_type swap(item_type item, location_type location) noexcept
        {
            item_type old_item = table_[location];
            table_[location] = item;
            return old_item;
        }

        /*
        Undoes the eviction chain recorded in eviction_journal_, restoring table_ to exactly the contents it had
        before the chain started, and returns the item the chain started from.

        Step k of the random walk is `item = swap(item, loc_k)`, which exchanges the item in flight with the contents
        of loc_k. That exchange is an involution, so applying it again at the same location undoes it. Undoing the
        steps in reverse order therefore composes the inverses in reverse order, which is the identity, and both the
        table and the item in flight are restored: the item that falls out of the last undone step is the item the
        caller passed to insert.

        Nothing in that argument requires the locations to be distinct. If the walk visits location L at steps 3 and
        9, the unwind writes L at step 9 first and at step 3 last, so the final content of L is what was there before
        the earliest visit. Later writes are undone before earlier ones, which is exactly what an undo log must do.
        */
        item_type unwind_eviction_chain(item_type item) noexcept
        {
            for (auto loc = eviction_journal_.crbegin(); loc != eviction_journal_.crend(); loc++)
            {
                item = swap(item, *loc);
            }
            return item;
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
        Storage for the item that the most recent failed insert could not place. Written only by the failure path of
        insert and by clear_table; deliberately not cleared on success, because callers depend on being able to read
        it after a batch of inserts. Because insert is atomic, this is the item the caller passed to the failing
        call.
        */
        item_type leftover_item_;

        /*
        Scratch space for insert: the table locations written by the random walk of the call in progress, in the
        order they were written, so that a walk which ends in failure can be undone. It is cleared at the very top
        of every call to insert, before any other work, so a previous call's contents can never be replayed. Between
        calls it may still hold the locations written by the previous call; nothing other than insert may read it.
        It is held as a member only so that its capacity is reused across calls. Its length is bounded by max_probe,
        with a payload of four bytes per entry plus whatever the vector over-allocates.
        */
        std::vector<location_type> eviction_journal_;

        /*
        The number of items that have been inserted to table or stash.
        */
        table_size_type inserted_items_ = 0;

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

        This value is only meaningful when found() is true. It is then an index into table() if in_stash() is false,
        and an index into stash() if in_stash() is true. When found() is false this function returns 0, which is
        indistinguishable from a genuine hit at table location 0, so callers must check found() (or the equivalent
        conversion to bool) before using the result. Prefer location_if_found(), which cannot be read without
        handling the not-found case.
        */
        [[nodiscard]] location_type location() const noexcept
        {
            return location_;
        }

        /**
        Returns the hash table or stash location represented by this QueryResult, or std::nullopt if the queried item
        was not found. This is the misuse-resistant form of location(): the not-found case cannot be mistaken for a
        hit at location 0. Use in_stash() to tell whether the returned index refers to table() or to stash().
        */
        [[nodiscard]] std::optional<location_type> location_if_found() const noexcept
        {
            if (!found())
            {
                return std::nullopt;
            }
            return location_;
        }

        /**
        Returns the index of the location function that was used to insert the queried item. This value is only
        meaningful when found() is true and in_stash() is false. It is equal to max_loc_func_count when the item was
        not found, and it is an unspecified sentinel when the item was found in the stash; in neither of those cases
        may it be used as a location function index. See the description of max_loc_func_count in common.h for the
        full encoding.
        */
        [[nodiscard]] std::uint32_t loc_func_index() const noexcept
        {
            return loc_func_index_;
        }

        /**
        Returns whether the queried item was found in the stash. When this is true, location() is an index into
        stash() rather than into table(), and loc_func_index() is meaningless.
        */
        [[nodiscard]] bool in_stash() const noexcept
        {
            return !~loc_func_index_;
        }

        /**
        Returns whether the queried item was found in the hash table or in the stash.
        */
        [[nodiscard]] bool found() const noexcept
        {
            return !(loc_func_index_ & ~(max_loc_func_count - 1)) || in_stash();
        }

        /**
        Returns whether the queried item was found in the hash table or in the stash.
        */
        operator bool() const noexcept
        {
            return found();
        }

    private:
        QueryResult(location_type location, std::uint32_t loc_func_index)
            : location_(location), loc_func_index_(loc_func_index)
        {}

        location_type location_ = 0;

        // Initialized to the "not found" sentinel so a default-constructed QueryResult reports
        // !found() — anything < max_loc_func_count would falsely report a hit at slot 0.
        std::uint32_t loc_func_index_ = max_loc_func_count;
    };
} // namespace kuku
