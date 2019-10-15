#pragma once
#include "../../src/kuku/kuku.h"
#include "../../src/kuku/common.h"

typedef struct
{
	std::size_t location;
	std::size_t loc_func_index;
	bool in_stash;
} QueryResult;

extern "C" __declspec(dllexport) kuku::KukuTable* kukuTable_Create(int log_table_size,
	std::size_t stash_size,
	std::size_t loc_func_count,
	kuku::item_type loc_func_seed,
	std::uint64_t max_probe,
	kuku::item_type empty_item);

extern "C" __declspec(dllexport) bool kukuTable_Insert(kuku::KukuTable* kuku_table, kuku::item_type item);

extern "C" __declspec(dllexport) bool kukuTable_Query(kuku::KukuTable* kuku_table, kuku::item_type item, QueryResult query_result);

extern "C" __declspec(dllexport) bool kukuTable_IsEmptyItem(kuku::KukuTable* kuku_table, kuku::item_type& item);

extern "C" __declspec(dllexport) bool kukuTable_LastInsertFailItem(kuku::KukuTable* kuku_table, uint64_t* item);

extern "C" __declspec(dllexport) double kukuTable_FillRate(kuku::KukuTable* kuku_table);

extern "C" __declspec(dllexport) bool kukuTable_Table(kuku::KukuTable* kuku_table, std::size_t index, uint64_t* item);

extern "C" __declspec(dllexport) std::size_t kukuTable_TableSize(kuku::KukuTable* kuku_table);

extern "C" __declspec(dllexport) bool kukuTable_Stash(kuku::KukuTable* kuku_table, std::size_t index, uint64_t* item);

extern "C" __declspec(dllexport) std::size_t kukuTable_StashSize(kuku::KukuTable* kuku_table);