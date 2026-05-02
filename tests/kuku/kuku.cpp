// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "kuku/kuku.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cmath>

using namespace kuku;
using namespace std;

namespace kuku_tests
{
    TEST(KukuTableTests, Create)
    {
        ASSERT_THROW(KukuTable(0, 0, 2, make_zero_item(), 1, make_zero_item()), invalid_argument);
        ASSERT_THROW(KukuTable(1, 0, 0, make_zero_item(), 1, make_zero_item()), invalid_argument);
        ASSERT_THROW(KukuTable(1, 0, 2, make_zero_item(), 0, make_zero_item()), invalid_argument);
        ASSERT_NO_THROW(KukuTable(min_table_size, 0, 2, make_zero_item(), 1, make_zero_item()));
        ASSERT_NO_THROW(KukuTable(1, 0, min_loc_func_count, make_zero_item(), 1, make_zero_item()));
    }

    TEST(KukuTableTests, Populate1)
    {
        {
            KukuTable ct(1, 0, 1, make_zero_item(), 10, make_zero_item());
            ASSERT_TRUE(ct.is_empty(0));
            ASSERT_TRUE(ct.insert(make_item(1, 0)));
            ASSERT_FALSE(ct.insert(make_item(0, 1)));
            ASSERT_THROW((void)ct.insert(ct.empty_item()), invalid_argument);
            ASSERT_THROW((void)ct.insert(make_item(0, 0)), invalid_argument);
            ASSERT_FALSE(ct.is_empty(0));
        }
        {
            KukuTable ct(1, 0, 2, make_zero_item(), 10, make_zero_item());
            ASSERT_TRUE(ct.is_empty(0));
            ASSERT_TRUE(ct.insert(make_item(1, 0)));
            ASSERT_FALSE(ct.insert(make_item(0, 1)));
            ASSERT_THROW((void)ct.insert(ct.empty_item()), invalid_argument);
            ASSERT_THROW((void)ct.insert(make_item(0, 0)), invalid_argument);
            ASSERT_FALSE(ct.is_empty(0));
        }
        {
            KukuTable ct(2, 0, 1, make_zero_item(), 10, make_zero_item());
            ASSERT_TRUE(ct.is_empty(0));
            ASSERT_TRUE(ct.insert(make_item(1, 0)));

            // Collision
            ASSERT_FALSE(ct.insert(make_item(0, 1)));

            // No collision
            ASSERT_TRUE(ct.insert(make_item(0, 2)));

            ASSERT_FALSE(ct.is_empty(0));
            ASSERT_FALSE(ct.is_empty(1));
        }
    }

    TEST(KukuTableTests, Populate2)
    {
        KukuTable ct(1U << 10U, 0, 2, make_zero_item(), 10, make_zero_item());
        for (location_type i = 0; i < ct.table_size(); i++)
        {
            ASSERT_TRUE(ct.is_empty(i));
        }

        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_TRUE(ct.insert(make_item(0, 1)));
        ASSERT_TRUE(ct.insert(make_item(1, 1)));
        ASSERT_TRUE(ct.insert(make_item(2, 2)));
        ASSERT_THROW((void)ct.insert(ct.empty_item()), invalid_argument);
        ASSERT_THROW((void)ct.insert(make_item(0, 0)), invalid_argument);

        int non_empties = 0;
        for (location_type i = 0; i < ct.table_size(); i++)
        {
            non_empties += ct.is_empty(i) ? 0 : 1;
        }
        ASSERT_EQ(non_empties, 4);

        ASSERT_TRUE(ct.query(make_item(1, 0)));
        ASSERT_TRUE(ct.query(make_item(0, 1)));
        ASSERT_TRUE(ct.query(make_item(1, 1)));
        ASSERT_TRUE(ct.query(make_item(2, 2)));
        ASSERT_FALSE(ct.query(make_item(3, 3)));
    }

    TEST(KukuTableTests, Populate3)
    {
        KukuTable ct(1U << 10U, 0, 2, make_zero_item(), 10, make_random_item());
        for (location_type i = 0; i < ct.table_size(); i++)
        {
            ASSERT_TRUE(ct.is_empty(i));
        }

        ASSERT_TRUE(ct.insert(make_item(0, 0)));
        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_TRUE(ct.insert(make_item(0, 1)));
        ASSERT_TRUE(ct.insert(make_item(1, 1)));
        ASSERT_TRUE(ct.insert(make_item(2, 2)));

        // Fails
        ASSERT_FALSE(ct.insert(make_item(2, 2)));

        ASSERT_THROW((void)ct.insert(ct.empty_item()), invalid_argument);

        int non_empties = 0;
        for (location_type i = 0; i < ct.table_size(); i++)
        {
            non_empties += ct.is_empty(i) ? 0 : 1;
        }
        ASSERT_EQ(non_empties, 5);

        ASSERT_TRUE(ct.query(make_item(0, 0)));
        ASSERT_TRUE(ct.query(make_item(1, 0)));
        ASSERT_TRUE(ct.query(make_item(0, 1)));
        ASSERT_TRUE(ct.query(make_item(1, 1)));
        ASSERT_TRUE(ct.query(make_item(2, 2)));
        ASSERT_FALSE(ct.query(make_item(3, 3)));
    }

    TEST(KukuTableTests, Fill1)
    {
        KukuTable ct(1U << 10U, 0, 2, make_zero_item(), 100, make_random_item());
        vector<item_type> inserted_items;
        for (int i = 0; i < 100; i++)
        {
            inserted_items.emplace_back(make_random_item());
            ASSERT_TRUE(ct.insert(inserted_items.back()));
        }
        for (auto b : inserted_items)
        {
            ASSERT_TRUE(ct.query(b));
        }
        ASSERT_FALSE(ct.query(make_random_item()));
    }

    TEST(KukuTableTests, Fill2)
    {
        KukuTable ct((1U << 10U) - 1, 0, 4, make_zero_item(), 100, make_random_item());
        vector<item_type> inserted_items;
        for (int i = 0; i < 600; i++)
        {
            inserted_items.emplace_back(make_random_item());
            ASSERT_TRUE(ct.insert(inserted_items.back()));
        }
        for (auto b : inserted_items)
        {
            ASSERT_TRUE(ct.query(b));
        }
        ASSERT_FALSE(ct.query(make_random_item()));
    }

    TEST(KukuTableTests, Fill3)
    {
        KukuTable ct((1U << 10U) + 1, 4, 2, make_zero_item(), 100, make_random_item());
        vector<item_type> inserted_items;
        for (int i = 0; i < 950; i++)
        {
            inserted_items.emplace_back(make_random_item());
            if (!ct.insert(inserted_items.back()))
            {
                auto it = find_if(inserted_items.cbegin(), inserted_items.cend(), [&](const item_type &item) {
                    return are_equal_item(ct.leftover_item(), item);
                });
                ASSERT_TRUE(it != inserted_items.cend());
                ASSERT_FALSE(ct.query(ct.leftover_item()));
                inserted_items.erase(it);
            }
        }
        for (auto b : inserted_items)
        {
            ASSERT_TRUE(ct.query(b));
        }
    }

    TEST(KukuTableTests, Locations)
    {
        uint8_t lfc = 2;
        KukuTable ct(1U << 10U, 4, lfc, make_random_item(), 100, make_all_ones_item());
        for (int k = 0; k < 20; k++)
        {
            auto it = make_random_item();
            auto all_locs = ct.all_locations(it);

            bool collision_found = false;
            for (uint8_t i = 0; i < lfc; i++)
            {
                for (uint8_t j = 0; j < i; j++)
                {
                    collision_found = collision_found || (ct.location(it, i) == ct.location(it, j));
                }
            }

            ASSERT_EQ(all_locs.size() < lfc, collision_found);
        }
    }

    TEST(KukuTableTests, RepeatedInsert)
    {
        KukuTable ct(1U << 10U, 0, 4, make_zero_item(), 10, make_zero_item());
        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_TRUE(ct.insert(make_item(0, 1)));
        ASSERT_TRUE(ct.insert(make_item(1, 1)));
        ASSERT_TRUE(ct.insert(make_item(2, 2)));

        ASSERT_FALSE(ct.insert(make_item(1, 0)));
        ASSERT_FALSE(ct.insert(make_item(0, 1)));
        ASSERT_FALSE(ct.insert(make_item(1, 1)));
        ASSERT_FALSE(ct.insert(make_item(2, 2)));
    }

    TEST(KukuTableTests, Accessors)
    {
        item_type seed = make_item(0xDEADBEEF, 0xCAFEBABE);
        item_type empty = make_all_ones_item();
        KukuTable ct(64, 4, 3, seed, 200, empty);

        ASSERT_EQ(64U, ct.table_size());
        ASSERT_EQ(4U, ct.stash_size());
        ASSERT_EQ(3U, ct.loc_func_count());
        ASSERT_EQ(200U, ct.max_probe());
        ASSERT_TRUE(are_equal_item(seed, ct.loc_func_seed()));
        ASSERT_TRUE(are_equal_item(empty, ct.empty_item()));
        ASSERT_TRUE(ct.is_empty_item(empty));
        ASSERT_FALSE(ct.is_empty_item(make_item(1, 1)));
        ASSERT_TRUE(are_equal_item(empty, ct.leftover_item()));
        ASSERT_DOUBLE_EQ(0.0, ct.fill_rate());
        ASSERT_EQ(64U, ct.table().size());

        ASSERT_TRUE(ct.insert(make_item(1, 1)));
        ASSERT_DOUBLE_EQ(1.0 / static_cast<double>(64 + 4), ct.fill_rate());
    }

    TEST(KukuTableTests, OutOfRange)
    {
        KukuTable ct(8, 2, 2, make_zero_item(), 10, make_zero_item());

        // table(index) throws when index >= table_size
        ASSERT_NO_THROW((void)ct.table(7));
        ASSERT_THROW((void)ct.table(8), out_of_range);
        ASSERT_THROW((void)ct.table(100), out_of_range);

        // stash(index) throws when index >= stash_size (note: stash_size, not stash().size())
        ASSERT_NO_THROW((void)ct.stash(0));
        ASSERT_NO_THROW((void)ct.stash(1));
        ASSERT_THROW((void)ct.stash(2), out_of_range);

        // location(item, lf_idx) throws when lf_idx >= loc_func_count
        ASSERT_NO_THROW((void)ct.location(make_item(1, 1), 0));
        ASSERT_NO_THROW((void)ct.location(make_item(1, 1), 1));
        ASSERT_THROW((void)ct.location(make_item(1, 1), 2), out_of_range);
        ASSERT_THROW((void)ct.location(make_item(1, 1), 100), out_of_range);

        // location and all_locations throw on the empty item
        ASSERT_THROW((void)ct.location(ct.empty_item(), 0), invalid_argument);
        ASSERT_THROW((void)ct.all_locations(ct.empty_item()), invalid_argument);

        // is_empty(index) bounds via table(index)
        ASSERT_NO_THROW((void)ct.is_empty(7));
        ASSERT_THROW((void)ct.is_empty(8), out_of_range);
    }

    TEST(KukuTableTests, ClearTable)
    {
        KukuTable ct(8, 2, 2, make_zero_item(), 10, make_zero_item());

        // Fill a few entries, including one that lands in the stash.
        size_t inserted = 0;
        for (uint64_t i = 1; i < 32 && inserted < 6; i++)
        {
            if (ct.insert(make_item(i, i + 1)))
            {
                inserted++;
            }
        }
        ASSERT_GT(inserted, 0U);
        ASSERT_GT(ct.fill_rate(), 0.0);

        ct.clear_table();

        ASSERT_DOUBLE_EQ(0.0, ct.fill_rate());
        ASSERT_TRUE(ct.is_empty_item(ct.leftover_item()));
        ASSERT_EQ(0U, ct.stash().size());
        for (table_size_type i = 0; i < ct.table_size(); i++)
        {
            ASSERT_TRUE(ct.is_empty(i));
        }

        // Re-inserting after clear works.
        ASSERT_TRUE(ct.insert(make_item(1, 1)));
    }

    TEST(KukuTableTests, QueryResultDefaultIsNotFound)
    {
        // Default-constructed QueryResult must report not-found; otherwise a stack-allocated
        // result that's never assigned would falsely indicate a hit at slot 0.
        QueryResult result;
        ASSERT_FALSE(result.found());
        ASSERT_FALSE(static_cast<bool>(result));
    }
} // namespace kuku_tests
