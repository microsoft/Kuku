#include "pch.h"
#include "kuku_ref.h"

extern "C" __declspec(dllexport) kuku::KukuTable* kukuTable_Create(int log_table_size,
	std::size_t stash_size,
	std::size_t loc_func_count,
	kuku::item_type loc_func_seed,
	std::uint64_t max_probe,
	kuku::item_type empty_item) {
	return new kuku::KukuTable(log_table_size,
		stash_size,
		loc_func_count,
		loc_func_seed,
		max_probe,
		empty_item);
}

extern "C" __declspec(dllexport) bool kukuTable_Insert(kuku::KukuTable* kuku_table, kuku::item_type item)
{
	return kuku_table->insert(item);
}

extern "C" __declspec(dllexport) bool kukuTable_Query(kuku::KukuTable* kuku_table, kuku::item_type item, QueryResult query_result)
{
	kuku::QueryResult res = kuku_table->query(item);
	query_result.location = res.location();
	query_result.loc_func_index = res.loc_func_index();
	query_result.in_stash = res.in_stash();
	return true;
}

extern "C" __declspec(dllexport) bool kukuTable_IsEmptyItem(kuku::KukuTable* kuku_table, kuku::item_type& item)
{
	return kuku_table->is_empty_item(item);
}

extern "C" __declspec(dllexport) bool kukuTable_LastInsertFailItem(kuku::KukuTable* kuku_table, uint64_t* item)
{
	kuku::item_type src = kuku_table->last_insert_fail_item();
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" __declspec(dllexport) double kukuTable_FillRate(kuku::KukuTable* kuku_table)
{
	return kuku_table->fill_rate();
}

extern "C" __declspec(dllexport) bool kukuTable_Table(kuku::KukuTable* kuku_table, std::size_t index, uint64_t* item)
{
	kuku::item_type src = kuku_table->table(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" __declspec(dllexport) std::size_t kukuTable_TableSize(kuku::KukuTable* kuku_table)
{
	return kuku_table->table_size();
}

extern "C" __declspec(dllexport) bool kukuTable_Stash(kuku::KukuTable* kuku_table, std::size_t index, uint64_t* item)
{
	kuku::item_type src = kuku_table->stash(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" __declspec(dllexport) std::size_t kukuTable_StashSize(kuku::KukuTable* kuku_table)
{
	return kuku_table->stash_size();
}

std::size_t kukuTable_Location(kuku::KukuTable* kuku_table, kuku::item_type item, std::size_t loc_func_index)
{
	return kuku_table->location(item, loc_func_index);
}