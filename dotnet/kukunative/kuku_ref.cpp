#include "pch.h"
#include "kuku_ref.h"

extern "C" EXPORT kuku::KukuTable* kukuTable_Create( 
	uint32_t table_size,
	uint32_t stash_size,
	uint32_t loc_func_count,
	uint64_t *loc_func_seed,
	uint64_t max_probe,
	uint64_t *empty_item) 
{
	kuku::item_type kuku_loc_func_seed = kuku::make_item(loc_func_seed[0], loc_func_seed[1]);
	kuku::item_type kuku_empty_item = kuku::make_item(empty_item[0], empty_item[1]);
	return new kuku::KukuTable(
		table_size,
		stash_size,
		loc_func_count,
		kuku_loc_func_seed,
		max_probe,
		kuku_empty_item);
}

extern "C" EXPORT bool kukuTable_Insert(kuku::KukuTable* kuku_table, uint64_t *item)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	return kuku_table->insert(kuku_item);
}

extern "C" EXPORT bool kukuTable_Query(kuku::KukuTable* kuku_table, uint64_t *item, QueryResult *query_result)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	kuku::QueryResult res = kuku_table->query(kuku_item);
	query_result->location = res.location();
	query_result->loc_func_index = res.loc_func_index();
	query_result->in_stash = res.in_stash();
	return true;
}

extern "C" EXPORT bool kukuTable_IsEmptyItem(kuku::KukuTable* kuku_table, uint64_t *item)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	return kuku_table->is_empty_item(kuku_item);
}

extern "C" EXPORT bool kukuTable_LastInsertFailItem(kuku::KukuTable* kuku_table, uint64_t* item)
{
	kuku::item_type src = kuku_table->last_insert_fail_item();
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" EXPORT double kukuTable_FillRate(kuku::KukuTable* kuku_table)
{
	return kuku_table->fill_rate();
}

extern "C" EXPORT bool kukuTable_Table(kuku::KukuTable* kuku_table, uint32_t index, uint64_t* item)
{
	kuku::item_type src = kuku_table->table(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" EXPORT uint32_t kukuTable_TableSize(kuku::KukuTable* kuku_table)
{
	return kuku_table->table_size();
}

extern "C" EXPORT bool kukuTable_Stash(kuku::KukuTable* kuku_table, uint32_t index, uint64_t* item)
{
	kuku::item_type src = kuku_table->stash(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

extern "C" EXPORT uint32_t kukuTable_StashSize(kuku::KukuTable* kuku_table)
{
	return kuku_table->stash_size();
}

extern "C" EXPORT uint32_t kukuTable_Location(kuku::KukuTable* kuku_table, uint64_t* item, uint32_t loc_func_index)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	return kuku_table->location(kuku_item, loc_func_index);
}