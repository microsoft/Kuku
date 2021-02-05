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
    if (argc != 5)
    {
        cout << "Usage: ./example table_size stash_size loc_func_count max_probe" << endl;
        cout << "E.g., ./example 256 2 4 100" << endl;

        return 0;
    }

    auto table_size = static_cast<table_size_type>(atoi(argv[1]));
    auto stash_size = static_cast<table_size_type>(atoi(argv[2]));
    uint8_t loc_func_count = static_cast<uint8_t>(atoi(argv[3]));
    item_type loc_func_seed = make_random_item();
    uint64_t max_probe = static_cast<uint64_t>(atoi(argv[4]));
    item_type empty_item = make_item(0, 0);

    KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item);

    uint64_t round_counter = 0;
    while (true)
    {
        cout << "Inserted " << round_counter * 20 << " items" << endl;
        cout << "Fill rate: " << table.fill_rate() << endl;

        char c;
        cin.get(c);

        for (uint64_t i = 0; i < 20; i++)
        {
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

        print_table(table);

        if (!table.is_empty_item(table.leftover_item()))
        {
            break;
        }

        round_counter++;
    }

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

    return 0;
}
