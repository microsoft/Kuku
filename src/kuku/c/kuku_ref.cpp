// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pch.h"
#include "kuku_ref.h"
#include <set>

KUKU_C_FUNC(void *) KukuTable_Create(
    uint32_t table_size, uint32_t stash_size, uint32_t loc_func_count, uint64_t *loc_func_seed, uint64_t max_probe,
    uint64_t *empty_item)
{
    kuku::item_type kuku_loc_func_seed = kuku::make_item(loc_func_seed[0], loc_func_seed[1]);
    kuku::item_type kuku_empty_item = kuku::make_item(empty_item[0], empty_item[1]);
    return new kuku::KukuTable(table_size, stash_size, loc_func_count, kuku_loc_func_seed, max_probe, kuku_empty_item);
}

KUKU_C_FUNC(bool) KukuTable_Insert(void *kuku_table, uint64_t *item)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
    bool b = ptr->insert(kuku_item);
    return b;
}

KUKU_C_FUNC(bool) KukuTable_Query(void *kuku_table, uint64_t *item, QueryResultData *query_result)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
    kuku::QueryResult res = ptr->query(kuku_item);
    query_result->found = !!res;
    query_result->in_stash = res.in_stash();
    query_result->location = res.location();
    query_result->loc_func_index = res.loc_func_index();
    return query_result->found;
}

KUKU_C_FUNC(bool) KukuTable_IsEmptyItem(void *kuku_table, uint64_t *item)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
    return ptr->is_empty_item(kuku_item);
}

KUKU_C_FUNC(void) KukuTable_EmptyItem(void *kuku_table, uint64_t *item)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = ptr->empty_item();
    item[0] = kuku::get_low_word(kuku_item);
    item[1] = kuku::get_high_word(kuku_item);
}

KUKU_C_FUNC(void) KukuTable_LeftoverItem(void *kuku_table, uint64_t *item)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = ptr->leftover_item();
    item[0] = kuku::get_low_word(kuku_item);
    item[1] = kuku::get_high_word(kuku_item);
}

KUKU_C_FUNC(double) KukuTable_FillRate(void *kuku_table)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    return ptr->fill_rate();
}

KUKU_C_FUNC(uint32_t) KukuTable_LocFuncCount(void *kuku_table)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    return ptr->loc_func_count();
}

KUKU_C_FUNC(void) KukuTable_Table(void *kuku_table, uint32_t index, uint64_t *item)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = ptr->table(index);
    item[0] = kuku::get_low_word(kuku_item);
    item[1] = kuku::get_high_word(kuku_item);
}

KUKU_C_FUNC(uint32_t) KukuTable_TableSize(void *kuku_table)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    return ptr->table_size();
}

KUKU_C_FUNC(void) KukuTable_Stash(void *kuku_table, uint32_t index, uint64_t *item)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = ptr->stash(index);
    item[0] = kuku::get_low_word(kuku_item);
    item[1] = kuku::get_high_word(kuku_item);
}

KUKU_C_FUNC(uint32_t) KukuTable_StashSize(void *kuku_table)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    return ptr->stash_size();
}

KUKU_C_FUNC(uint32_t) KukuTable_Location(void *kuku_table, uint64_t *item, uint32_t loc_func_index)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
    return ptr->location(kuku_item, loc_func_index);
}

KUKU_C_FUNC(void) KukuTable_AllLocations(void *kuku_table, uint64_t *item, uint32_t *locations, uint32_t *count)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
    std::set<kuku::location_type> loc_set = ptr->all_locations(kuku_item);
    for (auto loc : loc_set)
    {
        *locations++ = loc;
    }
    *count = static_cast<uint32_t>(loc_set.size());
}

KUKU_C_FUNC(void) KukuTable_ClearTable(void *kuku_table)
{
    auto ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
    ptr->clear_table();
}

KUKU_C_FUNC(void) Common_SetRandomItem(uint64_t *item)
{
    kuku::item_type kuku_item = kuku::make_random_item();
    item[0] = kuku::get_low_word(kuku_item);
    item[1] = kuku::get_high_word(kuku_item);
}

KUKU_C_FUNC(void) Common_IncrementItem(uint64_t *item)
{
    kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
    kuku::increment_item(kuku_item);
    item[0] = kuku::get_low_word(kuku_item);
    item[1] = kuku::get_high_word(kuku_item);
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
