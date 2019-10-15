// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "gtest/gtest.h"
#include "kuku/common.h"
#include <cmath>

using namespace kuku;
using namespace std;

namespace kuku_tests
{
    TEST(CommonTests, SetItem)
    {
        item_type bl;
        set_item(0, 0, bl);
        ASSERT_EQ(0, bl[0]);
        ASSERT_EQ(0, bl[1]);

        set_item(1, 0, bl);
        ASSERT_EQ(1, bl[0]);
        ASSERT_EQ(0, bl[1]);

        set_item(0, 1, bl);
        ASSERT_EQ(0, bl[0]);
        ASSERT_EQ(1, bl[1]);

        set_item(0xF00F, 0xBABA, bl);
        ASSERT_EQ(0xF00F, bl[0]);
        ASSERT_EQ(0xBABA, bl[1]);

        set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
        ASSERT_EQ(0xF00FF00FF00FF00F, bl[0]);
        ASSERT_EQ(0xBABABABABABABABA, bl[1]);

        unsigned char data[bytes_per_item]{
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07 };
        set_item(data, bl);
        ASSERT_EQ(0x0706050403020100, bl[0]);
        ASSERT_EQ(0x0706050403020100, bl[1]);
    }

    TEST(CommonTests, SetZeroItem)
    {
        item_type bl;
        set_item(0, 0, bl);
        set_zero_item(bl);
        ASSERT_EQ(0, bl[0]);
        ASSERT_EQ(0, bl[1]);

        set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
        set_zero_item(bl);
        ASSERT_EQ(0, bl[0]);
        ASSERT_EQ(0, bl[1]);
    }

    TEST(CommonTests, SetAllOnesItem)
    {
        item_type bl;
        set_zero_item(bl);
        set_all_ones_item(bl);
        ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, bl[0]);
        ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, bl[1]);

        set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
        set_all_ones_item(bl);
        ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, bl[0]);
        ASSERT_EQ(0xFFFFFFFFFFFFFFFFULL, bl[1]);
    }

    TEST(CommonTests, IsZeroItem)
    {
        item_type bl;
        set_item(0, 0, bl);
        ASSERT_TRUE(is_zero_item(bl));

        set_item(1, 0, bl);
        ASSERT_FALSE(is_zero_item(bl));

        set_item(0, 1, bl);
        ASSERT_FALSE(is_zero_item(bl));

        set_item(0xF00FF00FF00FF00F, 0xBABABABABABABABA, bl);
        ASSERT_FALSE(is_zero_item(bl));
    }

    TEST(CommonTests, IsAllOnesItem)
    {
        item_type bl;
        set_all_ones_item(bl);
        ASSERT_TRUE(is_all_ones_item(bl));

        set_item(0xFFFFFFFFFFFFFFFFULL, 0, bl);
        ASSERT_FALSE(is_all_ones_item(bl));

        set_item(0, 0xFFFFFFFFFFFFFFFFULL, bl);
        ASSERT_FALSE(is_all_ones_item(bl));

        set_item(0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL, bl);
        ASSERT_FALSE(is_all_ones_item(bl));
    }

    TEST(CommonTests, SetRandomItem)
    {
        item_type bl;
        set_random_item(bl);
        ASSERT_FALSE(is_zero_item(bl));
        item_type bl2 = bl;
        ASSERT_TRUE(are_equal_item(bl, bl2));
        set_random_item(bl);
        ASSERT_FALSE(is_zero_item(bl));
        ASSERT_FALSE(are_equal_item(bl, bl2));
    }

    TEST(CommonTests, ZeroItem)
    {
        item_type bl = make_random_item();
        ASSERT_FALSE(is_zero_item(bl));
        bl = make_zero_item();
        ASSERT_TRUE(is_zero_item(bl));
    }

    TEST(CommonTests, IncrementItem)
    {
        item_type it = make_item(0, 0);
        increment_item(it);
        ASSERT_EQ(1, it[0]);
        ASSERT_EQ(0, it[1]);

        it = make_item(0xF00F, 0xBAAB);
        increment_item(it);
        ASSERT_EQ(0xF010, it[0]);
        ASSERT_EQ(0xBAAB, it[1]);

        it = make_item(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE);
        increment_item(it);
        ASSERT_EQ(0x0, it[0]);
        ASSERT_EQ(0xFFFFFFFFFFFFFFFF, it[1]);

        it = make_item(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
        increment_item(it);
        ASSERT_EQ(0x0, it[0]);
        ASSERT_EQ(0x0, it[1]);
    }
}
