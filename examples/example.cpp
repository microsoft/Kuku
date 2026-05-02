// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <iomanip>
#include <iostream>
#include <kuku/kuku.h>
#include <string>

using namespace std;
using namespace kuku;

void print_table(const KukuTable &table)
{
    table_size_type col_count = 8;
    for (table_size_type i = 0; i < table.table_size(); i++)
    {
        const auto &item = table.table(i);
        cout << setw(5) << i << ": " << setw(5) << get_high_word(item) << "," << get_low_word(item)
             << ((i % col_count == col_count - 1) ? "\n" : "\t");
    }

    cout << "\n\nStash: \n";
    for (table_size_type i = 0; i < table.stash().size(); i++)
    {
        const auto &item = table.stash(i);
        cout << i << ": " << get_high_word(item) << "," << get_low_word(item) << "\n";
    }
    cout << "\n";
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 5)
        {
            cout << "Usage: ./example table_size stash_size loc_func_count max_probe\n";
            cout << "E.g., ./example 256 2 4 100\n";
            return 0;
        }

        auto table_size = static_cast<table_size_type>(stoul(argv[1]));
        auto stash_size = static_cast<table_size_type>(stoul(argv[2]));
        auto loc_func_count = static_cast<uint8_t>(stoul(argv[3]));
        item_type loc_func_seed = make_random_item();
        auto max_probe = static_cast<uint64_t>(stoull(argv[4]));
        item_type empty_item = make_item(0, 0);

        KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item);

        uint64_t round_counter = 0;
        while (true)
        {
            cout << "Inserted " << round_counter * 20 << " items\n";
            cout << "Fill rate: " << table.fill_rate() << "\n";

            char c = 0;
            cin.get(c);

            for (uint64_t i = 0; i < 20; i++)
            {
                if (!table.insert(make_item(i + 1, round_counter + 1)))
                {
                    cout << "Insertion failed: round_counter = " << round_counter << ", i = " << i << "\n";
                    cout << "Inserted successfully " << ((round_counter * 20) + i) << " items\n";
                    cout << "Fill rate: " << table.fill_rate() << "\n";
                    const auto &item = table.leftover_item();
                    cout << "Leftover item: " << get_high_word(item) << "," << get_low_word(item) << "\n\n";
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
            string hw;
            string lw;
            if (!getline(cin, hw, ',') || !getline(cin, lw, '\n'))
            {
                break;
            }
            item_type item = make_item(static_cast<uint64_t>(stoull(lw)), static_cast<uint64_t>(stoull(hw)));
            QueryResult res = table.query(item);
            cout << "Found: " << boolalpha << !!res << "\n";
            if (res)
            {
                cout << "Location: " << res.location() << "\n";
                cout << "In stash: " << boolalpha << res.in_stash() << "\n";
                cout << "Hash function index: " << res.loc_func_index() << "\n\n";
            }
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
