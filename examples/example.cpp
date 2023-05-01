// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <chrono>
#include <iomanip>
#include <iostream>
#include <kuku/kuku.h>

using namespace std;
using namespace kuku;

ostream &operator<<(ostream &stream, item_type item)
{
    stream << item[1] << " " << item[0];
    return stream;
}
//Print table command borrowed from microsoft's example file
void print_table(const KukuTable &table)
{
    table_size_type col_count = 8;
    for (table_size_type i = 0; i < table.table_size(); i++)
    {
        const auto &item = table.table(i);
        cout << setw(5)
            << i << ": " << setw(5) << get_high_word(item) << "," << get_low_word(item)
            << ((i % col_count == col_count - 1) ? "\n" : "\t");
    }
    //our examples will likely not use a stash
    if(table.stash_size() > 0) {
        cout << endl << endl << "Stash: " << endl;
        for (table_size_type i = 0; i < table.stash().size(); i++)
        {
            const auto &item = table.stash(i);
            cout << i << ": " << get_high_word(item) << "," << get_low_word(item) << endl;
        }
        cout << endl;
    }
}

double get_fill_rate(table_size_type, table_size_type, table_size_type, uint8_t, uint64_t, uint64_t);

int main(int argc, char *argv[])
{
    /* Tables will have the following properties
     * 1000000 available indexes 
     * no stash allowed
     * bucketSizes between 2 and 5
     * between 2 and 5 hash functions
     * a limit of 100 swaps before the table declares an error, this results in 
     * rehashing
     */
    table_size_type tableSizes = 1000000;
    table_size_type stashSizes = 0;
    table_size_type bucketSizes[] = {1, 2, 3, 4, 5};
    uint8_t hashFunctions[] = {2, 3, 4, 5};
    uint64_t swapLimit = 100;
    uint64_t insertions = 1000000;

    const int bucketListLength = 5, hashListLength = 4;
    double fillRates[bucketListLength][hashListLength];

    for(int bucketIndex = 0; bucketIndex < bucketListLength; ++bucketIndex) {
        for(int hashIndex = 0; hashIndex < hashListLength; ++hashIndex) {
            fillRates[bucketIndex][hashIndex] = get_fill_rate(tableSizes, stashSizes, bucketSizes[bucketIndex], hashFunctions[hashIndex], swapLimit, insertions);
        }
    }

    return 0;
}

double get_fill_rate(
        table_size_type table_size, 
        table_size_type stash_size, 
        table_size_type bucketSize,
        uint8_t loc_func_count, 
        uint64_t max_probe,
        uint64_t insertions
    ) {
    item_type loc_func_seed = make_random_item();
    item_type empty_item = make_item(0, 0);

    KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item, bucketSize);

    uint64_t insertions_failed = 0;

    for(uint64_t inserted = 0; inserted < insertions; ++inserted) {
        if (!table.insert(make_item(inserted + 1, (inserted + 1) % 20 ))) {
            insertions_failed++;
            continue;
        }   
 
    }
    cout << "Buckets : " << bucketSize << ", Hash Count : " << (int) loc_func_count << ", Fill Rates : " << table.fill_rate() << ", Failed Insertions : " << insertions_failed << endl;
        
    return table.fill_rate();
}