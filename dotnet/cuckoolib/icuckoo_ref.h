#pragma once
#include "../src/cuckoo/common.h"
#include "../src/cuckoo/cuckoo.h"

class ICuckoo
{
public: 
	virtual bool insert(cuckoo::item_type item) = 0;
	virtual cuckoo::QueryResult query(cuckoo::item_type item) const = 0;
};