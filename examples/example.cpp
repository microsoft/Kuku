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

    cout << endl << endl << "Stash: " << endl;
    for (table_size_type i = 0; i < table.stash().size(); i++)
    {
        const auto &item = table.stash(i);
        cout << i << ": " << get_high_word(item) << "," << get_low_word(item) << endl;
    }
    cout << endl;
}

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
    table_size_type tableSizes = 80;
    table_size_type stashSizes = 0;
    table_size_type bucketSizes[] = {2, 3, 4, 5};
    uint8_t hashFunctions[] = {2, 3, 4, 5};
    uint64_t swapLimit = 100;
    uint64_t insertions = 20;

    const int bucketListLength = 5, hashListLength = 5;
    double fillRates[bucketListLength][hashListLength];

    for(int bucketIndex = 0; bucketIndex < bucketListLength; ++bucketIndex) {
        for(int hashIndex = 0; hashIndex < hashListLength; ++hashIndex) {
            fillRates[bucketIndex][hashIndex] = get_fill_rate(tableSizes, stashSizes, bucketSizes[bucketIndex], hashFunctions[hashIndex], swapLimit, insertions);
        }
    }
    //printf the fill rates
    for(int bucketIndex = 0; bucketIndex < bucketListLength; ++bucketIndex) {
        for(int hashIndex = 0; hashIndex < hashListLength; ++hashIndex) {
            cout << "Buckets : " << bucketSizes[bucketIndex] << ", Hash Count : " << hashFunctions[hashIndex] << ", Fill Rates : " << fillRates[bucketIndex][hashIndex] << endl;
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

    uint64_t round_counter = 0;

    for(uint64_t inserted = 0; inserted < insertions; ++inserted) {
        if (!table.insert(make_item(inserted + 1, (inserted + 1) % 20 ))) {
            cout << "Insert Failed on Insertion " << inserted << endl;
            cout << "Fill rate: " << table.fill_rate() << endl;
            const auto &item = table.leftover_item();
            cout << "Leftover item: " << get_high_word(item) << "," << get_low_word(item) << endl << endl;
            break;
        }
        
    }
    print_table(table);
/* Old insertion method
    while (false)
    {
        cout << "Inserted " << round_counter * 20 << " items" << endl;
        cout << "Fill rate: " << table.fill_rate() << endl;

        for (uint64_t i = 0; i < 20; i++)
        {
            cout << " " << i ; 
            if (!table.insert(make_item(i + 1, round_counter + 1)))
            {
                cout << "Insertion failed: round_counter = " << round_counter << ", i = " << i << endl;
                cout << "Inserted successfully " << round_counter * 20 + i << " items" << endl;
                cout << "Fill rate: " << table.fill_rate() << endl;
                const auto &item = table.leftover_item();
                cout << "Leftover item: " << get_high_word(item) << "," << get_low_word(item) << endl << endl;
                break;
            }
        }
    
        if (!table.is_empty_item(table.leftover_item()))
        {
            break;
        }

        round_counter++;
    } */

/* Option to Query items
    while (true)
    {
        cout << "Query item: ";
        char hw[64];
        char lw[64];
        cin.getline(hw, 10, ',');
        cin.getline(lw, 10, '\n');
        item_type item = make_item(static_cast<uint64_t>(atoi(lw)), static_cast<uint64_t>(atoi(hw)));
        QueryResult res = table.query(item);
        cout << "Found: " << boolalpha << !!res << endl;
        if (res)
        {
            cout << "Location: " << res.location() << endl;
            cout << "In stash: " << boolalpha << res.in_stash() << endl;
            cout << "Hash function index: " << res.loc_func_index() << endl << endl;
        }
    }
 */
    return table.fill_rate();
}