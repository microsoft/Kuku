// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "gtest/gtest.h"
#include "kuku/common.h"
#include "kuku/locfunc.h"
#include <cmath>

using namespace kuku;
using namespace std;

namespace kuku_tests
{
    TEST(LocFuncTests, Create)
    {
        ASSERT_THROW(LocFunc(0, make_item(0, 0)), invalid_argument);
        ASSERT_THROW(LocFunc(-1, make_item(0, 0)), invalid_argument);
        ASSERT_THROW(LocFunc(0, make_item(1, 1)), invalid_argument);
    }

    TEST(LocFuncTests, Randomness)
    {
        random_device rd;
        for (int ts = min_table_size; ts < 5 * min_table_size; ts++)
        {
            for (int attempts = 0; attempts < 10; attempts++)
            {
                uint64_t seed_lw = rd();
                uint64_t seed_hw = rd();
                LocFunc lf(ts, make_item(seed_lw, seed_hw));
                LocFunc lf2(ts, make_item(seed_lw, seed_hw));

                uint64_t zeros = 0;
                uint64_t total = 1000;
                for (uint64_t i = 0; i < total; i++)
                {
                    item_type bl = make_item(rd(), rd());
                    ASSERT_TRUE(lf(bl) == lf2(bl));
                    zeros += (lf(bl) == size_t(0));
                }
                ASSERT_TRUE(abs(static_cast<double>(zeros) / static_cast<double>(total) - pow(2.0, -ts)) < 0.05);
            }
        }
    }
}
