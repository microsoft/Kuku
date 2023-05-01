// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <chrono>
#include <iomanip>
#include <iostream>
#include <kuku/kuku.h>
#include <sys/time.h>
#include<array>

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
    cout << endl;
}

double get_fill_rate(table_size_type, table_size_type, table_size_type, uint8_t, uint64_t, uint64_t, bool);

int main(int argc, char *argv[])
{
    /* Tables will have the following properties
     * 1000000 available indexes 
     * no stash allowed
     * bucketSizes between 1 and 5
     * between 2 and 5 hash functions
     * a limit of 100 swaps before the table declares an error, this results in 
     * rehashing
     */
    table_size_type tableSizes = 4000000;
    table_size_type stashSizes = 0;
    table_size_type bucketSizes[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t hashFunctions[] = {2, 3, 4, 5, 6, 7, 8};
    uint64_t swapLimit = 10;
    uint64_t insertions = 4000000;

    const int bucketListLength = sizeof(bucketSizes) / sizeof(bucketSizes[0]), hashListLength = sizeof(hashFunctions) / sizeof(hashFunctions[0]);
    double fillRates[bucketListLength][hashListLength];
    /* 
    cout << "Do not end after failure" << endl;
    for(int bucketIndex = 0; bucketIndex < bucketListLength; ++bucketIndex) {
        for(int hashIndex = 0; hashIndex < hashListLength; ++hashIndex) {
            fillRates[bucketIndex][hashIndex] = get_fill_rate(tableSizes, stashSizes, bucketSizes[bucketIndex], hashFunctions[hashIndex], swapLimit, insertions, false);
        }
        cout<< endl;
    }
 */
    cout << "End after need for rehash" << endl;
    for(int bucketIndex = 0; bucketIndex < bucketListLength; ++bucketIndex) {
        for(int hashIndex = 0; hashIndex < hashListLength; ++hashIndex) {
            fillRates[bucketIndex][hashIndex] = get_fill_rate(tableSizes, stashSizes, bucketSizes[bucketIndex], hashFunctions[hashIndex], swapLimit, insertions, true);
        }
        cout<< endl;
    }
    return 0;
}
//creates and outputs various data on the hash table
double get_fill_rate(
        table_size_type table_size, 
        table_size_type stash_size, 
        table_size_type bucketSize,
        uint8_t loc_func_count, 
        uint64_t max_probe,
        uint64_t insertions,
        bool endOnFail
    ) {
    item_type loc_func_seed = make_random_item();
    item_type empty_item = make_item(0, 0);

    KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item, bucketSize);

    uint64_t insertions_failed = 0, inserted;
    struct timeval time;
    gettimeofday(&time, NULL);
    double startTime = (double) time.tv_sec + (double) time.tv_usec * 0.000001;
    for(inserted = 0; inserted < insertions; ++inserted) {
        if (!table.insert(make_item(inserted + 1, (uint64_t) rand() ))) {
            insertions_failed++;
            if(endOnFail) break;
            else continue;
        }   
 
    }
    gettimeofday(&time, NULL);
    double totalTime = ((double) time.tv_sec + (double) time.tv_usec * 0.000001) - startTime;
    cout << "Bucket Size : " << bucketSize << ", Hash Count : " << (int) loc_func_count << ", Fill Rates : " << table.fill_rate() << ", Percent Inserted : " << (double) inserted / insertions <<", Wall Time : " << totalTime << endl;
    //if(loc_func_count == 2 && bucketSize == 3) {print_table(table);};   
    return table.fill_rate();
}