// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/kuku.h"
#include <cstddef>
#include <cstdint>

// Check that std::size_t is 64 bits
static_assert(sizeof(std::size_t) == 8, "Require sizeof(std::size_t) == 8");

#ifdef _MSC_VER

// Check that architecture (platform) is x64
#ifndef _WIN64
static_assert(false, "Require architecture == x64");
#endif

#if defined(KUKU_C_EXPORTS) || defined(kuku_c_EXPORTS) || defined(kukuc_EXPORTS)
#define KUKU_C_DECOR extern "C" __declspec(dllexport)
#else
#define KUKU_C_DECOR extern "C" __declspec(dllimport)
#endif

#define KUKU_C_CALL __cdecl

#else // _MSC_VER

#define KUKU_C_DECOR extern "C"
#define KUKU_C_CALL

#endif // _MSC_VER

#define KUKU_C_FUNC(x) KUKU_C_DECOR x KUKU_C_CALL

typedef struct
{
    bool found;
    bool in_stash;
    uint32_t location;
    uint32_t loc_func_index;
} QueryResultData;

KUKU_C_FUNC(void *)
KukuTable_Create(
    uint32_t table_size, uint32_t stash_size, uint32_t loc_func_count, uint64_t *loc_func_seed, uint64_t max_probe,
    uint64_t *empty_item);

KUKU_C_FUNC(bool) KukuTable_Insert(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(bool) KukuTable_Query(void *kuku_table, uint64_t *item, QueryResultData *query_result);

KUKU_C_FUNC(bool) KukuTable_IsEmptyItem(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(void) KukuTable_EmptyItem(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(void) KukuTable_LeftoverItem(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(double) KukuTable_FillRate(void *kuku_table);

KUKU_C_FUNC(uint32_t) KukuTable_LocFuncCount(void *kuku_table);

KUKU_C_FUNC(void) KukuTable_Table(void *kuku_table, uint32_t index, uint64_t *item);

KUKU_C_FUNC(uint32_t) KukuTable_TableSize(void *kuku_table);

KUKU_C_FUNC(void) KukuTable_Stash(void *kuku_table, uint32_t index, uint64_t *item);

KUKU_C_FUNC(uint32_t) KukuTable_StashSize(void *kuku_table);

KUKU_C_FUNC(uint32_t) KukuTable_Location(void *kuku_table, uint64_t *item, uint32_t loc_func_index);

KUKU_C_FUNC(void) KukuTable_AllLocations(void *kuku_table, uint64_t *item, uint32_t *locations, uint32_t *count);

KUKU_C_FUNC(void) KukuTable_ClearTable(void *kuku_table);

KUKU_C_FUNC(void) Common_SetRandomItem(uint64_t *item);

KUKU_C_FUNC(void) Common_IncrementItem(uint64_t *item);

KUKU_C_FUNC(uint32_t) Common_MinTableSize();

KUKU_C_FUNC(uint32_t) Common_MaxTableSize();

KUKU_C_FUNC(uint32_t) Common_MinLocFuncCount();

KUKU_C_FUNC(uint32_t) Common_MaxLocFuncCount();
