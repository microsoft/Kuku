// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <algorithm>
#include <iterator>
#include "pch.h"
#include "kuku_ref.h"

KUKU_C_FUNC(kuku::KukuTable*) KukuTable_Create( 
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

KUKU_C_FUNC(bool) KukuTable_Insert(kuku::KukuTable *kuku_table, uint64_t *item)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	return kuku_table->insert(kuku_item);
}

KUKU_C_FUNC(bool) KukuTable_Query(kuku::KukuTable *kuku_table, uint64_t *item, QueryResult *query_result)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	kuku::QueryResult res = kuku_table->query(kuku_item);
	query_result->found = !!res;
	query_result->in_stash = res.in_stash();
	query_result->location = res.location();
	query_result->loc_func_index = res.loc_func_index();
	return true;
}

KUKU_C_FUNC(bool) KukuTable_IsEmptyItem(kuku::KukuTable *kuku_table, uint64_t *item)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	return kuku_table->is_empty_item(kuku_item);
}

KUKU_C_FUNC(bool) KukuTable_LastInsertFailItem(kuku::KukuTable *kuku_table, uint64_t *item)
{
	kuku::item_type src = kuku_table->last_insert_fail_item();
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

KUKU_C_FUNC(double) KukuTable_FillRate(kuku::KukuTable *kuku_table)
{
	return kuku_table->fill_rate();
}

KUKU_C_FUNC(bool) KukuTable_Table(kuku::KukuTable *kuku_table, uint32_t index, uint64_t *item)
{
	kuku::item_type src = kuku_table->table(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

KUKU_C_FUNC(uint32_t) KukuTable_TableSize(kuku::KukuTable *kuku_table)
{
	return kuku_table->table_size();
}

KUKU_C_FUNC(bool) KukuTable_Stash(kuku::KukuTable *kuku_table, uint32_t index, uint64_t *item)
{
	kuku::item_type src = kuku_table->stash(index);
	std::copy(std::begin(src), std::end(src), item);
	return true;
}

KUKU_C_FUNC(uint32_t) KukuTable_StashSize(kuku::KukuTable *kuku_table)
{
	return kuku_table->stash_size();
}

KUKU_C_FUNC(uint32_t) KukuTable_Location(kuku::KukuTable *kuku_table, uint64_t *item, uint32_t loc_func_index)
{
	kuku::item_type kuku_item = kuku::make_item(item[0], item[1]);
	return kuku_table->location(kuku_item, loc_func_index);
}