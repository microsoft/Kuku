// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// The C ABI header is included by its full path, not as "kuku.h": this directory now also
// sits next to src/kuku/kuku.h, and a quoted include would resolve to whichever the compiler
// finds first.
#include "kuku/c/kuku.h"
#include "kuku/kuku.h"
#include <cstddef>
#include <cstdint>
#include <set>

// ABI invariants. These live here rather than in kuku/c/kuku.h because that header must stay
// parsable by a C compiler, which has neither std:: nor kuku::. This is the translation unit
// that implements the ABI, so a violation is still caught exactly where it is introduced.
static_assert(sizeof(std::size_t) == 8, "Require sizeof(std::size_t) == 8");

// The C ABI exposes table sizes/locations as uint32_t. If table_size_type ever
// changes, the C ABI silently truncates without this assertion.
static_assert(sizeof(kuku::table_size_type) == sizeof(uint32_t), "table_size_type must be 32 bits for the C ABI");
static_assert(sizeof(kuku::location_type) == sizeof(uint32_t), "location_type must be 32 bits for the C ABI");

// Throwing across an extern "C" boundary is UB. Every entry point that calls
// into a potentially-throwing C++ function must catch internally and report
// failure via the return value (null handle / false / zeroed output).
//
// Every entry point that takes a caller-supplied pointer must null-check it;
// otherwise a null argument leads to UB before the catch block runs, defeating
// the boundary protection.

KUKU_C_FUNC(void *)
KukuTable_Create(
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
    {}
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

KUKU_C_FUNC(bool) KukuTable_Query(void *kuku_table, uint64_t *item, KukuQueryResult *query_result)
{
    if (nullptr == query_result)
    {
        return false;
    }

    // Put the output into the defined "not found" state before anything else, so that every
    // failure path below leaves it identical rather than leaving a caller's uninitialized
    // stack struct untouched. loc_func_index must be max_loc_func_count and NOT zero:
    // QueryResult::found() treats any index below max_loc_func_count as a hit, so zero would
    // report a spurious find at location 0.
    query_result->found = false;
    query_result->in_stash = false;
    query_result->location = 0;
    query_result->loc_func_index = kuku::max_loc_func_count;

    if (nullptr == kuku_table || nullptr == item)
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
        query_result->loc_func_index = kuku::max_loc_func_count;
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
    if (nullptr == item)
    {
        return;
    }
    // Defined output on every failure path; see KukuTable_Query.
    item[0] = 0;
    item[1] = 0;
    if (nullptr == kuku_table)
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
    if (nullptr == item)
    {
        return;
    }
    // Defined output on every failure path; see KukuTable_Query.
    item[0] = 0;
    item[1] = 0;
    if (nullptr == kuku_table)
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

KUKU_C_FUNC(bool) KukuTable_Table(void *kuku_table, uint32_t index, uint64_t *item)
{
    if (nullptr == item)
    {
        return false;
    }
    // Defined output on every failure path; see KukuTable_Query.
    item[0] = 0;
    item[1] = 0;
    if (nullptr == kuku_table)
    {
        return false;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = ptr->table(index);
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
        return true;
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
        return false;
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

KUKU_C_FUNC(bool) KukuTable_Stash(void *kuku_table, uint32_t index, uint64_t *item)
{
    if (nullptr == item)
    {
        return false;
    }
    // Defined output on every failure path; see KukuTable_Query.
    item[0] = 0;
    item[1] = 0;
    if (nullptr == kuku_table)
    {
        return false;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = ptr->stash(index);
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
        return true;
    }
    catch (...)
    {
        item[0] = 0;
        item[1] = 0;
        return false;
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

// Status-returning counterpart to KukuTable_Location. That function reports failure as 0,
// which is indistinguishable from a legitimate location 0, so a caller that does not
// pre-validate its arguments silently acts on the wrong slot. This variant reports success
// explicitly and writes the location through an out-parameter. Prefer it in new code.
KUKU_C_FUNC(bool)
KukuTable_TryLocation(void *kuku_table, uint64_t *item, uint32_t loc_func_index, uint32_t *location)
{
    if (nullptr == location)
    {
        return false;
    }
    // Defined output on every failure path, as elsewhere in this ABI.
    *location = 0;
    if (nullptr == kuku_table || nullptr == item)
    {
        return false;
    }
    try
    {
        auto *ptr = reinterpret_cast<kuku::KukuTable *>(kuku_table);
        kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
        *location = ptr->location(kuku_item, loc_func_index);
        return true;
    }
    catch (...)
    {
        *location = 0;
        return false;
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
    {}
}

KUKU_C_FUNC(bool) Kuku_SetRandomItem(uint64_t *item)
{
    if (nullptr == item)
    {
        return false;
    }
    try
    {
        // make_random_item() is not noexcept: std::random_device can fail on platforms where
        // the entropy source is unavailable. MSVC's implementation calls rand_s and raises
        // via _Xout_of_range (std::out_of_range) rather than std::system_error, so the catch
        // below is deliberately unqualified.
        kuku::item_type kuku_item = kuku::make_random_item();
        item[0] = kuku::get_low_word(kuku_item);
        item[1] = kuku::get_high_word(kuku_item);
        return true;
    }
    catch (...)
    {
        // Entropy failure. The output is zeroed so that it is never left indeterminate, but
        // callers MUST check the return value: an all-zero item is a perfectly legal value,
        // so a caller that ignores the result cannot tell a random item from a failure and
        // would silently proceed with a fixed, publicly known location-function seed.
        item[0] = 0;
        item[1] = 0;
        return false;
    }
}

KUKU_C_FUNC(void) Kuku_IncrementItem(uint64_t *item)
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

KUKU_C_FUNC(uint32_t) Kuku_MinTableSize(void)
{
    return kuku::min_table_size;
}

KUKU_C_FUNC(uint32_t) Kuku_MaxTableSize(void)
{
    return kuku::max_table_size;
}

KUKU_C_FUNC(uint32_t) Kuku_MinStashSize(void)
{
    return kuku::min_stash_size;
}

KUKU_C_FUNC(uint32_t) Kuku_MaxStashSize(void)
{
    return kuku::max_stash_size;
}

KUKU_C_FUNC(uint32_t) Kuku_MinLocFuncCount(void)
{
    return kuku::min_loc_func_count;
}

KUKU_C_FUNC(uint32_t) Kuku_MaxLocFuncCount(void)
{
    return kuku::max_loc_func_count;
}
