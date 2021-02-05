// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "kuku/common.h"
#include "kuku/locfunc.h"
#include "gtest/gtest.h"
#include <cmath>

using namespace kuku;
using namespace std;

namespace kuku_tests
{
    TEST(LocFuncTests, Create)
    {
        ASSERT_THROW(LocFunc(0, make_item(0, 0)), invalid_argument);
        ASSERT_THROW(LocFunc(max_table_size + 1, make_item(0, 0)), invalid_argument);
        ASSERT_THROW(LocFunc(0, make_item(1, 1)), invalid_argument);
    }

    TEST(LocFuncTests, Randomness)
    {
        for (table_size_type ts = min_table_size; ts < 5 * min_table_size; ts++)
        {
            for (int attempts = 0; attempts < 10; attempts++)
            {
                item_type seed = make_random_item();
                LocFunc lf(ts, seed);
                LocFunc lf2(ts, seed);

                uint64_t zeros = 0;
                uint64_t total = 1000;
                for (uint64_t i = 0; i < total; i++)
                {
                    item_type bl;
                    set_random_item(bl);
                    ASSERT_TRUE(lf(bl) == lf2(bl));
                    zeros += (lf(bl) == size_t(0));
                }
                ASSERT_TRUE(
                    abs(static_cast<double>(zeros) / static_cast<double>(total) - 1 / static_cast<double>(ts)) < 0.05);
            }
        }
    }
} // namespace kuku_tests
