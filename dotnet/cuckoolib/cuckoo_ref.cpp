#include "pch.h"
#include "cuckoo_ref.h"

extern "C" __declspec(dllexport) cuckoo::CuckooTable* CuckooTable_Create(int log_table_size,
	std::size_t stash_size,
	std::size_t loc_func_count,
	cuckoo::item_type loc_func_seed,
	std::uint64_t max_probe,
	cuckoo::item_type empty_item) {
	return new cuckoo::CuckooTable(log_table_size,
		stash_size,
		loc_func_count,
		loc_func_seed,
		max_probe,
		empty_item);
}

extern "C" __declspec(dllexport) bool CuckooTable_Insert(cuckoo::CuckooTable* cuckoo_table, cuckoo::item_type item)
{
	return cuckoo_table->insert(item);
}

extern "C" __declspec(dllexport) bool CuckooTable_Query(cuckoo::CuckooTable* cuckoo_table, cuckoo::item_type item, QueryResult query_result)
{
	cuckoo::QueryResult res = cuckoo_table->query(item);
	query_result.location = res.location();
	query_result.loc_func_index = res.loc_func_index();
	query_result.in_stash = res.in_stash();
	return true;
}

extern "C" __declspec(dllexport) bool CuckooTable_IsEmptyItem(cuckoo::CuckooTable* cuckoo_table, cuckoo::item_type& item)
{
	return cuckoo_table->is_empty_item(item);
}

extern "C" __declspec(dllexport) bool CuckooTable_LastInsertFailItem(cuckoo::CuckooTable* cuckoo_table, uint64_t* item)
{
	cuckoo::item_type src = cuckoo_table->last_insert_fail_item();
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" __declspec(dllexport) double CuckooTable_FillRate(cuckoo::CuckooTable* cuckoo_table)
{
	return cuckoo_table->fill_rate();
}

extern "C" __declspec(dllexport) bool CuckooTable_Table(cuckoo::CuckooTable* cuckoo_table, std::size_t index, uint64_t* item)
{
	cuckoo::item_type src = cuckoo_table->table(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" __declspec(dllexport) std::size_t CuckooTable_TableSize(cuckoo::CuckooTable* cuckoo_table)
{
	return cuckoo_table->table_size();
}

extern "C" __declspec(dllexport) bool CuckooTable_Stash(cuckoo::CuckooTable* cuckoo_table, std::size_t index, uint64_t* item)
{
	cuckoo::item_type src = cuckoo_table->stash(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" __declspec(dllexport) std::size_t CuckooTable_StashSize(cuckoo::CuckooTable* cuckoo_table)
{
	return cuckoo_table->stash_size();
}