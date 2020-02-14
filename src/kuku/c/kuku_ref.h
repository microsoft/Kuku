// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "kuku/kuku.h"
#include "kuku/common.h"
#include <cstddef>

#ifdef _MSC_VER
     // Check that architecture (platform) is x64
#    ifndef _WIN64
         static_assert(false, "Require architecture == x64");
#    endif
#    ifdef KUKU_C_EXPORTS
#        define KUKU_C_DECOR extern "C" __declspec(dllexport)
#    else
#        define KUKU_C_DECOR extern "C" __declspec(dllimport)
#    endif
#    define KUKU_C_CALL __cdecl
#else // _MSC_VER
#    define KUKU_C_DECOR extern "C"
#    define KUKU_C_CALL
#endif // _MSC_VER

#define KUKU_C_FUNC(x) KUKU_C_DECOR x KUKU_C_CALL

// Check that std::size_t is 64 bits
static_assert(sizeof(std::size_t) == 8, "Require sizeof(std::size_t) == 8");

typedef struct
{
	bool found;
	bool in_stash;
	uint32_t location;
	uint32_t loc_func_index;
} QueryResult;

KUKU_C_FUNC(kuku::KukuTable*) KukuTable_Create(
	uint32_t log_table_size,
	uint32_t stash_size,
	uint32_t loc_func_count,
	uint64_t *loc_func_seed,
	uint64_t max_probe,
	uint64_t *empty_item);

KUKU_C_FUNC(bool) KukuTable_Insert(kuku::KukuTable *kuku_table, uint64_t *item);

KUKU_C_FUNC(bool) KukuTable_Query(kuku::KukuTable *kuku_table, uint64_t *item, QueryResult *query_result);

KUKU_C_FUNC(bool) KukuTable_IsEmptyItem(kuku::KukuTable *kuku_table, uint64_t *item);

KUKU_C_FUNC(bool) KukuTable_LastInsertFailItem(kuku::KukuTable *kuku_table, uint64_t *item);

KUKU_C_FUNC(double) KukuTable_FillRate(kuku::KukuTable *kuku_table);

KUKU_C_FUNC(bool) KukuTable_Table(kuku::KukuTable *kuku_table, uint32_t index, uint64_t *item);

KUKU_C_FUNC(uint32_t) KukuTable_TableSize(kuku::KukuTable *kuku_table);

KUKU_C_FUNC(bool) KukuTable_Stash(kuku::KukuTable *kuku_table, uint32_t index, uint64_t *item);

KUKU_C_FUNC(uint32_t) KukuTable_StashSize(kuku::KukuTable *kuku_table);

KUKU_C_FUNC(uint32_t) KukuTable_Location(kuku::KukuTable *kuku_table, uint64_t *item, uint32_t loc_func_index);
