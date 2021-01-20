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
            ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
            ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);
            ASSERT_FALSE(ct.is_empty(0));
        }
        {
            KukuTable ct(1, 0, 2, make_zero_item(), 10, make_zero_item());
            ASSERT_TRUE(ct.is_empty(0));
            ASSERT_TRUE(ct.insert(make_item(1, 0)));
            ASSERT_FALSE(ct.insert(make_item(0, 1)));
            ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
            ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);
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
        KukuTable ct(1 << 10, 0, 2, make_zero_item(), 10, make_zero_item());
        for (location_type i = 0; i < ct.table_size(); i++)
        {
            ASSERT_TRUE(ct.is_empty(i));
        }

        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_TRUE(ct.insert(make_item(0, 1)));
        ASSERT_TRUE(ct.insert(make_item(1, 1)));
        ASSERT_TRUE(ct.insert(make_item(2, 2)));
        ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);
        ASSERT_THROW(ct.insert(make_item(0, 0)), invalid_argument);

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
        KukuTable ct(1 << 10, 0, 2, make_zero_item(), 10, make_random_item());
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

        ASSERT_THROW(ct.insert(ct.empty_item()), invalid_argument);

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
        KukuTable ct(1 << 10, 0, 2, make_zero_item(), 100, make_random_item());
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
        KukuTable ct((1 << 10) - 1, 0, 4, make_zero_item(), 100, make_random_item());
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
        KukuTable ct((1 << 10) + 1, 4, 2, make_zero_item(), 100, make_random_item());
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
        KukuTable ct(1 << 10, 4, lfc, make_random_item(), 100, make_all_ones_item());
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
        KukuTable ct(1 << 10, 0, 4, make_zero_item(), 10, make_zero_item());
        ASSERT_TRUE(ct.insert(make_item(1, 0)));
        ASSERT_TRUE(ct.insert(make_item(0, 1)));
        ASSERT_TRUE(ct.insert(make_item(1, 1)));
        ASSERT_TRUE(ct.insert(make_item(2, 2)));

        ASSERT_FALSE(ct.insert(make_item(1, 0)));
        ASSERT_FALSE(ct.insert(make_item(0, 1)));
        ASSERT_FALSE(ct.insert(make_item(1, 1)));
        ASSERT_FALSE(ct.insert(make_item(2, 2)));
    }
} // namespace kuku_tests
