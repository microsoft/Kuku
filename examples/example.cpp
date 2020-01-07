// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <kuku/kuku.h>
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace kuku;

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;


ostream &operator <<(ostream &stream, item_type item)
{
    stream << item[1] << " " << item[0];
    return stream;
}

void print_table(const KukuTable &table)
{
    table_size_type col_count = 8;
    for (table_size_type row = 0; row < table.table_size() / col_count; row++)
    {
        for (table_size_type col = 0; col < col_count; col++)
        {
            table_size_type index = row * col_count + col;
            cout << setw(5) << index << ": " << setw(5) << table.table(index) << "\t";
        }
        cout << endl;
    }

    cout << endl << "Stash: " << endl;
    for (table_size_type i = 0; i < table.stash().size(); i++)
    {
        cout << i << ": " << table.stash(i) << endl;
    }
    cout << endl;
}

int main(int argc, char *argv[])
{
    //if (argc != 5)
    //{
    //    cout << "Usage: ./example log_table_size stash_size loc_func_count max_probe" << endl;
    //    cout << "E.g., ./example 8 2 4 100" << endl;

    //    return 0;
    //}

    //int log_table_size = atoi(argv[1]);
    //table_size_type stash_size = static_cast<size_t>(atoi(argv[2]));
    //size_t loc_func_count = static_cast<size_t>(atoi(argv[3]));
    //item_type loc_func_seed = make_random_item();
    //uint64_t max_probe = static_cast<uint64_t>(atoi(argv[4]));
    //item_type empty_item = make_item(0, 0);

    //KukuTable table(
    //    log_table_size,
    //    stash_size,
    //    loc_func_count,
    //    loc_func_seed,
    //    max_probe,
    //    empty_item
    //);

    //uint64_t round_counter = 0;
    //while (true)
    //{
    //    cout << "Inserted " << round_counter * 20 << " items" << endl;
    //    cout << "Fill rate: " << table.fill_rate() << endl;

    //    char c;
    //    cin.get(c);

    //    for (uint64_t i = 0; i < 20; i++)
    //    {
    //        if (!table.insert(make_item(i + 1, round_counter + 1)))
    //        {
    //            cout << "Insertion failed: round_counter = "
    //                << round_counter << ", i = " << i << endl;
    //            cout << "Inserted successfully " << round_counter * 20 + i << " items" << endl;
    //            cout << "Fill rate: " << table.fill_rate() << endl;
    //            cout << "Leftover item: " << table.last_insert_fail_item() << endl << endl;
    //            break;
    //        }
    //    }

    //    print_table(table);

    //    if (!table.is_empty_item(table.last_insert_fail_item()))
    //    {
    //        break;
    //    }

    //    round_counter++;
    //}

    //while (true)
    //{
    //    cout << "Query item: ";
    //    char hw[64];
    //    char lw[64];
    //    cin.getline(hw, 10, ' ');
    //    cin.getline(lw, 10, '\n');
    //    item_type item = make_item(
    //        static_cast<uint64_t>(atoi(lw)), static_cast<uint64_t>(atoi(hw)));
    //    QueryResult res = table.query(item);
    //    cout << "Found: " << boolalpha << !!res << endl;
    //    cout << "Location: " << res.location() << endl;
    //    cout << "In stash: " << boolalpha << res.in_stash() << endl;
    //    cout << "Hash function index: " << res.loc_func_index() << endl << endl;
    //}

    for (int i = 0; i < 1; i++)
    {
        int num_items = 1 << 20;
        table_size_type stash_size = 2;
        size_t loc_func_count = 3;
        item_type loc_func_seed = make_random_item();
        uint64_t max_probe = 200;
        item_type empty_item = make_item(0, 0);

        KukuTable table(
            ceil(num_items * 1.2),
            stash_size,
            loc_func_count,
            loc_func_seed,
            max_probe,
            empty_item
        );

        chrono::high_resolution_clock clk;
        auto tp_start = clk.now();

        vector<item_type> items(num_items); 
        for (int i = 0; i < num_items; i++) {
            items[i] = make_item(i+1,i+1);
        }

        auto t1 = high_resolution_clock::now();
        for (uint64_t i = 0; i < num_items; i++)
        {
            if (!table.insert(items[i]))
            {
                cout << "Insertion failed: i = " << i << endl;
                // cout << "Inserted successfully " << round_counter * 20 + i << " items" << endl;
                cout << "Fill rate: " << table.fill_rate() << endl;
                cout << "Leftover item: " << table.last_insert_fail_item() << endl << endl;
                break;
            }
        }
        auto t2 = high_resolution_clock::now();
        std::cout << duration_cast<duration<double>>(t2 - t1).count() << std::endl;

        auto tp_end = clk.now();
        auto tp_diff = chrono::duration_cast<chrono::milliseconds>(tp_end - tp_start);
        cout << "Time: " << tp_diff.count() << " ms" << endl;
    }

    char c; 
    cin.get(c);

    return 0;
}
