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
            cout << "Usage: ./kukuexamples table_size stash_size loc_func_count max_probe\n";
            cout << "table_size must be between " << min_table_size << " and " << max_table_size << "\n";
            cout << "stash_size must be between " << min_stash_size << " and " << max_stash_size << "\n";
            cout << "loc_func_count must be between " << min_loc_func_count << " and " << max_loc_func_count << "\n";
            cout << "E.g., ./kukuexamples 256 2 4 100\n";
            return 0;
        }

        auto table_size = static_cast<table_size_type>(stoul(argv[1]));
        auto stash_size = static_cast<table_size_type>(stoul(argv[2]));
        auto loc_func_count = static_cast<uint32_t>(stoul(argv[3]));
        item_type loc_func_seed = make_random_item();
        auto max_probe = static_cast<uint64_t>(stoull(argv[4]));
        item_type empty_item = make_item(0, 0);

        KukuTable table(table_size, stash_size, loc_func_count, loc_func_seed, max_probe, empty_item);

        uint64_t round_counter = 0;
        bool insertion_failed = false;
        while (true)
        {
            cout << "Inserted " << round_counter * 20 << " items\n";
            cout << "Fill rate: " << table.fill_rate() << "\n";

            char c = 0;
            cin.get(c);

            for (uint64_t i = 0; i < 20; i++)
            {
                item_type item = make_item(i + 1, round_counter + 1);
                if (!table.insert(item))
                {
                    // insert is atomic: the table and the stash are exactly as they were before
                    // this call, and the item that could not be placed is the one that was passed
                    // in. No earlier item was displaced, so there is nothing to recover and
                    // nothing to re-insert; the caller only has to record that this one item was
                    // not stored. Every item for which insert returned true is still findable.
                    insertion_failed = true;
                    cout << "Insertion failed: round_counter = " << round_counter << ", i = " << i << "\n";
                    cout << "Inserted successfully " << ((round_counter * 20) + i) << " items\n";
                    cout << "Fill rate: " << table.fill_rate() << "\n";
                    cout << "The table is unchanged. The item that could not be inserted is " << get_high_word(item)
                         << "," << get_low_word(item) << "\n";
                    cout << "leftover_item() reports the same item: " << boolalpha
                         << are_equal_item(item, table.leftover_item()) << "\n\n";
                    break;
                }
            }

            print_table(table);

            if (insertion_failed)
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
