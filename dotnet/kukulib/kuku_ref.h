#pragma once
#include "../../src/kuku/kuku.h"
#include "../../src/kuku/common.h"

#if defined(_MSC_VER)
//  Microsoft 
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define EXPORT
#define IMPORT
#else
//  Do nothing
#define EXPORT
#define IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

typedef struct
{
	std::size_t location;
	std::size_t loc_func_index;
	bool in_stash;
} QueryResult;

extern "C" EXPORT kuku::KukuTable* kukuTable_Create(int log_table_size,
	std::size_t stash_size,
	std::size_t loc_func_count,
	uint64_t *loc_func_seed,
	uint64_t max_probe,
	uint64_t *empty_item);

extern "C" EXPORT bool kukuTable_Insert(kuku::KukuTable* kuku_table, uint64_t *item);

extern "C" EXPORT bool kukuTable_Query(kuku::KukuTable* kuku_table, uint64_t *item, QueryResult *query_result);

extern "C" EXPORT bool kukuTable_IsEmptyItem(kuku::KukuTable* kuku_table, uint64_t *item);

extern "C" EXPORT bool kukuTable_LastInsertFailItem(kuku::KukuTable* kuku_table, uint64_t* item);

extern "C" EXPORT double kukuTable_FillRate(kuku::KukuTable* kuku_table);

extern "C" EXPORT bool kukuTable_Table(kuku::KukuTable* kuku_table, std::size_t index, uint64_t* item);

extern "C" EXPORT std::size_t kukuTable_TableSize(kuku::KukuTable* kuku_table);

extern "C" EXPORT bool kukuTable_Stash(kuku::KukuTable* kuku_table, std::size_t index, uint64_t* item);

extern "C" EXPORT std::size_t kukuTable_StashSize(kuku::KukuTable* kuku_table);

extern "C" EXPORT std::size_t kukuTable_Location(kuku::KukuTable* kuku_table, uint64_t *item, std::size_t loc_func_index);
