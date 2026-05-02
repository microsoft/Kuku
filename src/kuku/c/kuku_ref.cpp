// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "kuku_ref.h"
#include <set>

// Throwing across an extern "C" boundary is UB. Every entry point that calls
// into a potentially-throwing C++ function must catch internally and report
// failure via the return value (null handle / false / zeroed output).
//
// Every entry point that takes a caller-supplied pointer must null-check it;
// otherwise a null argument leads to UB before the catch block runs, defeating
// the boundary protection.

KUKU_C_FUNC(void *) KukuTable_Create(
    uint32_t table_size, uint32_t stash_size, uint32_t loc_func_count, uint64_t *loc_func_seed, uint64_t max_probe,
    uint64_t *empty_item)
{
    if (nullptr == loc_func_seed || nullptr == empty_item)
    {
        return nullptr;
    }
    try
    {
        kuku::item_type kuku_loc_func_seed = kuku::make_item(loc_func_seed[0], loc_func_seed[1]);
        kuku::item_type kuku_empty_item = kuku::make_item(empty_item[0], empty_item[1]);
        // Ownership transferred to the C caller as an opaque void*; KukuTable_Destroy frees it.
        return new kuku::KukuTable( // NOLINT(cppcoreguidelines-owning-memory)
            table_size, stash_size, loc_func_count, kuku_loc_func_seed, max_probe, kuku_empty_item);
    }
    catch (...)
    {
        return nullptr;
    }
}

KUKU_C_FUNC(void) KukuTable_Destroy(void *kuku_table)
{
    // Frees memory that KukuTable_Create handed out as an opaque void*.
    try
    {
        delete reinterpret_cast<kuku::KukuTable *>(kuku_table); // NOLINT(cppcoreguidelines-owning-memory)
    }
    catch (...) // NOLINT(bugprone-empty-catch) - C ABI cannot propagate exceptions to the caller.
    {
    }
}

KUKU_C_FUNC(bool) KukuTable_Insert(void *kuku_table, uint64_t *item)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return false;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        return ptr->insert(kuku_item);
    }
    catch (...)
    {
        return false;
    }
}

KUKU_C_FUNC(bool) KukuTable_Query(void *kuku_table, uint64_t *item, QueryResultData *query_result)
{
    if (nullptr == kuku_table || nullptr == item || nullptr == query_result)
    {
        return false;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        kuku::QueryResult res = ptr->query(kuku_item);
        query_result->found = !!res;
        query_result->in_stash = res.in_stash();
        query_result->location = res.location();
        query_result->loc_func_index = res.loc_func_index();
        return query_result->found;
    }
    catch (...)
    {
        query_result->found = false;
        query_result->in_stash = false;
        query_result->location = 0;
        query_result->loc_func_index = 0;
        return false;
    }
}

KUKU_C_FUNC(bool) KukuTable_IsEmptyItem(void *kuku_table, uint64_t *item)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return false;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        return ptr->is_empty_item(kuku_item);
    }
    catch (...)
    {
        return false;
    }
}

KUKU_C_FUNC(void) KukuTable_EmptyItem(void *kuku_table, uint64_t *item)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = ptr->empty_item();
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
    }
}

KUKU_C_FUNC(void) KukuTable_LeftoverItem(void *kuku_table, uint64_t *item)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = ptr->leftover_item();
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
    }
}

KUKU_C_FUNC(double) KukuTable_FillRate(void *kuku_table)
{
    if (nullptr == kuku_table)
    {
        return 0.0;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        return ptr->fill_rate();
    }
    catch (...)
    {
        return 0.0;
    }
}

KUKU_C_FUNC(uint32_t) KukuTable_LocFuncCount(void *kuku_table)
{
    if (nullptr == kuku_table)
    {
        return 0;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        return ptr->loc_func_count();
    }
    catch (...)
    {
        return 0;
    }
}

KUKU_C_FUNC(void) KukuTable_Table(void *kuku_table, uint32_t index, uint64_t *item)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = ptr->table(index);
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
    }
}

KUKU_C_FUNC(uint32_t) KukuTable_TableSize(void *kuku_table)
{
    if (nullptr == kuku_table)
    {
        return 0;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        return ptr->table_size();
    }
    catch (...)
    {
        return 0;
    }
}

KUKU_C_FUNC(void) KukuTable_Stash(void *kuku_table, uint32_t index, uint64_t *item)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = ptr->stash(index);
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
    }
}

KUKU_C_FUNC(uint32_t) KukuTable_StashSize(void *kuku_table)
{
    if (nullptr == kuku_table)
    {
        return 0;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        return ptr->stash_size();
    }
    catch (...)
    {
        return 0;
    }
}

KUKU_C_FUNC(uint32_t) KukuTable_Location(void *kuku_table, uint64_t *item, uint32_t loc_func_index)
{
    if (nullptr == kuku_table || nullptr == item)
    {
        return 0;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        return ptr->location(kuku_item, loc_func_index);
    }
    catch (...)
    {
        return 0;
    }
}

// `count` is in/out: on entry, it is the capacity (in uint32_t elements) of
// `locations`; on return, it is the number of locations written. If the buffer
// is too small, no locations are written and `*count` is set to the required
// size, allowing the caller to retry with a larger buffer.
KUKU_C_FUNC(void) KukuTable_AllLocations(void *kuku_table, uint64_t *item, uint32_t *locations, uint32_t *count)
{
    if (nullptr == count)
    {
        return;
    }
    if (nullptr == kuku_table || nullptr == item || nullptr == locations)
    {
        *count = 0;
        return;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        std::set<kuku::location_type> loc_set = ptr->all_locations(kuku_item);
        const auto required = static_cast<uint32_t>(loc_set.size());
        if (*count < required)
        {
            *count = required;
            return;
        }
        uint32_t i = 0;
        for (auto loc : loc_set)
        {
            locations[i++] = loc;
        }
        *count = required;
    }
    catch (...)
    {
        *count = 0;
    }
}

KUKU_C_FUNC(void) KukuTable_ClearTable(void *kuku_table)
{
    if (nullptr == kuku_table)
    {
        return;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        ptr->clear_table();
    }
    catch (...) // NOLINT(bugprone-empty-catch) - C ABI has no return channel for failure.
    {
    }
}

KUKU_C_FUNC(void) Common_SetRandomItem(uint64_t *item)
{
    if (nullptr == item)
    {
        return;
    }
    try
    {
        // make_random_item() is not noexcept: std::random_device::operator() can throw
        // std::system_error on platforms where the entropy source is unavailable.
        kuku::item_type kuku_item = kuku::make_random_item();
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
    }
}

KUKU_C_FUNC(void) Common_IncrementItem(uint64_t *item)
{
    if (nullptr == item)
    {
        return;
    }
    try
    {
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        kuku::increment_item(kuku_item);
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
    }
}

KUKU_C_FUNC(uint32_t) Common_MinTableSize()
{
    return kuku::min_table_size;
}

KUKU_C_FUNC(uint32_t) Common_MaxTableSize()
{
    return kuku::max_table_size;
}

KUKU_C_FUNC(uint32_t) Common_MinLocFuncCount()
{
    return kuku::min_loc_func_count;
}

KUKU_C_FUNC(uint32_t) Common_MaxLocFuncCount()
{
    return kuku::max_loc_func_count;
}
