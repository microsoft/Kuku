#pragma once
#include "../../src/cuckoo/cuckoo.h"
#include "../../src/cuckoo/common.h"

typedef struct
{
	std::size_t location;
	std::size_t loc_func_index;
	bool in_stash;
} QueryResult;

extern "C" __declspec(dllexport) cuckoo::CuckooTable* CuckooTable_Create(int log_table_size,
	std::size_t stash_size,
	std::size_t loc_func_count,
	cuckoo::item_type loc_func_seed,
	std::uint64_t max_probe,
	cuckoo::item_type empty_item);

extern "C" __declspec(dllexport) bool CuckooTable_Insert(cuckoo::CuckooTable* cuckoo_table, cuckoo::item_type item);

extern "C" __declspec(dllexport) bool CuckooTable_Query(cuckoo::CuckooTable* cuckoo_table, cuckoo::item_type item, QueryResult query_result);

extern "C" __declspec(dllexport) bool CuckooTable_IsEmptyItem(cuckoo::CuckooTable* cuckoo_table, cuckoo::item_type& item);

extern "C" __declspec(dllexport) bool CuckooTable_LastInsertFailItem(cuckoo::CuckooTable* cuckoo_table, uint64_t* item);

extern "C" __declspec(dllexport) double CuckooTable_FillRate(cuckoo::CuckooTable* cuckoo_table);

extern "C" __declspec(dllexport) bool CuckooTable_Table(cuckoo::CuckooTable* cuckoo_table, std::size_t index, uint64_t* item);

extern "C" __declspec(dllexport) std::size_t CuckooTable_TableSize(cuckoo::CuckooTable* cuckoo_table);

extern "C" __declspec(dllexport) bool CuckooTable_Stash(cuckoo::CuckooTable* cuckoo_table, std::size_t index, uint64_t* item);

extern "C" __declspec(dllexport) std::size_t CuckooTable_StashSize(cuckoo::CuckooTable* cuckoo_table);