// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// Tests for the kukuc C ABI.
//
// These link the kukuc SHARED library and call it through kuku.h, so the
// exported symbols, calling convention and dllimport declarations are exercised
// exactly as an external consumer sees them. The C++ core is deliberately NOT
// linked into this target: everything below crosses the C boundary.
//
// The emphasis is on failure paths, because the C ABI encodes failure in-band and
// nothing else in the repository covers it. Note that several catch blocks in
// kuku.cpp guard operations that are noexcept and so cannot be provoked with a
// valid handle; those are called out below rather than faked.

#include "kuku/c/kuku.h"
#include "gtest/gtest.h"
#include <cstring>
#include <vector>

using namespace std;

namespace kuku_c_tests
{
    namespace
    {
        constexpr uint32_t default_table_size = 512;
        constexpr uint32_t default_stash_size = 4;
        constexpr uint32_t default_loc_func_count = 3;
        constexpr uint64_t default_max_probe = 100;

        // An all-ones sentinel, matching the convention used by the C++ tests.
        void set_all_ones(uint64_t *item)
        {
            item[0] = ~uint64_t(0);
            item[1] = ~uint64_t(0);
        }

        void *make_default_table()
        {
            uint64_t seed[2] = { 0, 0 };
            uint64_t empty[2];
            set_all_ones(empty);
            return KukuTable_Create(
                default_table_size, default_stash_size, default_loc_func_count, seed, default_max_probe, empty);
        }

        // Fill an output with a recognizable bit pattern, so a test can distinguish
        // "the callee wrote a defined value" from "the callee wrote nothing".
        void poison(KukuQueryResult &q)
        {
            memset(&q, 0xCD, sizeof(q));
        }

        void poison(uint64_t *item)
        {
            item[0] = 0xDEADBEEFDEADBEEFULL;
            item[1] = 0xFEEDFACEFEEDFACEULL;
        }
    } // namespace

    TEST(KukuCTests, ConstantAccessors)
    {
        ASSERT_EQ(1U, Kuku_MinTableSize());
        ASSERT_EQ(uint32_t(1) << 30, Kuku_MaxTableSize());
        ASSERT_EQ(0U, Kuku_MinStashSize());
        ASSERT_EQ(128U, Kuku_MaxStashSize());
        ASSERT_EQ(1U, Kuku_MinLocFuncCount());
        ASSERT_EQ(32U, Kuku_MaxLocFuncCount());

        // The bounds must be self-consistent, since callers use them to pre-validate.
        ASSERT_LE(Kuku_MinTableSize(), Kuku_MaxTableSize());
        ASSERT_LE(Kuku_MinStashSize(), Kuku_MaxStashSize());
        ASSERT_LE(Kuku_MinLocFuncCount(), Kuku_MaxLocFuncCount());
    }

    TEST(KukuCTests, CreateRejectsInvalidParameters)
    {
        uint64_t seed[2] = { 0, 0 };
        uint64_t empty[2];
        set_all_ones(empty);

        // Construction failure is reported as a null handle, which is unambiguous
        // because no valid table is ever represented by null.
        ASSERT_EQ(nullptr, KukuTable_Create(0, 0, 2, seed, 1, empty));
        ASSERT_EQ(nullptr, KukuTable_Create(Kuku_MaxTableSize() + 1, 0, 2, seed, 1, empty));
        ASSERT_EQ(nullptr, KukuTable_Create(1024, Kuku_MaxStashSize() + 1, 2, seed, 1, empty));
        ASSERT_EQ(nullptr, KukuTable_Create(1024, 0, 0, seed, 1, empty));
        ASSERT_EQ(nullptr, KukuTable_Create(1024, 0, Kuku_MaxLocFuncCount() + 1, seed, 1, empty));
        ASSERT_EQ(nullptr, KukuTable_Create(1024, 0, 2, seed, 0, empty));

        // Boundary values must be accepted.
        void *t = KukuTable_Create(Kuku_MinTableSize(), Kuku_MinStashSize(), Kuku_MinLocFuncCount(), seed, 1, empty);
        ASSERT_NE(nullptr, t);
        KukuTable_Destroy(t);

        t = KukuTable_Create(1024, Kuku_MaxStashSize(), 2, seed, 1, empty);
        ASSERT_NE(nullptr, t);
        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, CreateRejectsNullPointers)
    {
        uint64_t buf[2] = { 0, 0 };
        ASSERT_EQ(nullptr, KukuTable_Create(1024, 0, 2, nullptr, 1, buf));
        ASSERT_EQ(nullptr, KukuTable_Create(1024, 0, 2, buf, 1, nullptr));
    }

    TEST(KukuCTests, InsertAndQueryRoundTrip)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t item[2] = { 42, 43 };
        ASSERT_TRUE(KukuTable_Insert(t, item));

        KukuQueryResult q;
        poison(q);
        ASSERT_TRUE(KukuTable_Query(t, item, &q));
        ASSERT_TRUE(q.found);
        ASSERT_FALSE(q.in_stash);
        ASSERT_LT(q.loc_func_index, Kuku_MaxLocFuncCount());
        ASSERT_LT(q.location, default_table_size);

        // The item really is at the reported location.
        uint64_t slot[2];
        poison(slot);
        ASSERT_TRUE(KukuTable_Table(t, q.location, slot));
        ASSERT_EQ(item[0], slot[0]);
        ASSERT_EQ(item[1], slot[1]);

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, QueryMissReportsNotFoundSentinel)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t missing[2] = { 999999, 999999 };
        KukuQueryResult q;
        poison(q);
        ASSERT_FALSE(KukuTable_Query(t, missing, &q));
        ASSERT_FALSE(q.found);
        ASSERT_FALSE(q.in_stash);
        // Must be the not-found sentinel and NOT zero: zero is a valid location
        // function index and would represent a hit.
        ASSERT_EQ(Kuku_MaxLocFuncCount(), q.loc_func_index);

        KukuTable_Destroy(t);
    }

    // Regression test for the defect where the null-argument guard returned without
    // touching the caller's output struct, leaving an uninitialized stack struct
    // indeterminate while the exception path zeroed it.
    TEST(KukuCTests, QueryLeavesOutputDefinedOnEveryFailurePath)
    {
        uint64_t item[2] = { 1, 2 };

        KukuQueryResult q;
        poison(q);
        ASSERT_FALSE(KukuTable_Query(nullptr, item, &q));
        ASSERT_FALSE(q.found);
        ASSERT_FALSE(q.in_stash);
        ASSERT_EQ(0U, q.location);
        ASSERT_EQ(Kuku_MaxLocFuncCount(), q.loc_func_index);

        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        poison(q);
        ASSERT_FALSE(KukuTable_Query(t, nullptr, &q));
        ASSERT_FALSE(q.found);
        ASSERT_EQ(Kuku_MaxLocFuncCount(), q.loc_func_index);

        // The empty item is rejected by the core, which the wrapper turns into a
        // defined "not found" rather than letting the exception escape extern "C".
        uint64_t empty[2];
        set_all_ones(empty);
        poison(q);
        ASSERT_FALSE(KukuTable_Query(t, empty, &q));
        ASSERT_FALSE(q.found);
        ASSERT_EQ(Kuku_MaxLocFuncCount(), q.loc_func_index);

        // A null output pointer must simply be ignored, not dereferenced.
        ASSERT_FALSE(KukuTable_Query(t, item, nullptr));

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, ItemOutputsAreDefinedOnFailurePaths)
    {
        uint64_t out[2];

        // Null handle: each of these must overwrite the caller's buffer rather than
        // leaving stale contents behind.
        poison(out);
        KukuTable_EmptyItem(nullptr, out);
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        poison(out);
        KukuTable_LeftoverItem(nullptr, out);
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        poison(out);
        ASSERT_FALSE(KukuTable_Table(nullptr, 0, out));
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        poison(out);
        ASSERT_FALSE(KukuTable_Stash(nullptr, 0, out));
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        // Null output pointers must be ignored rather than dereferenced.
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);
        KukuTable_EmptyItem(t, nullptr);
        KukuTable_LeftoverItem(t, nullptr);
        ASSERT_FALSE(KukuTable_Table(t, 0, nullptr));
        ASSERT_FALSE(KukuTable_Stash(t, 0, nullptr));
        KukuTable_Destroy(t);
    }

    // A successful read and a failed read both produce {0, 0} when the slot happens to hold
    // the empty item, so only the status return distinguishes them. This is the whole point
    // of giving these functions a status channel.
    TEST(KukuCTests, TableAndStashReportStatusDistinctFromContents)
    {
        uint64_t seed[2] = { 0, 0 };
        uint64_t zero_empty[2] = { 0, 0 };
        // Deliberately choose {0, 0} as the empty item: it is the exact value the failure
        // path also writes, which is what makes status and contents indistinguishable here.
        void *t = KukuTable_Create(64, 2, 2, seed, 10, zero_empty);
        ASSERT_NE(nullptr, t);

        uint64_t out[2];

        // In range and empty: succeeds, and the contents are {0, 0}.
        poison(out);
        ASSERT_TRUE(KukuTable_Table(t, 0, out));
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        // Out of range: fails, and the contents are also {0, 0}. Identical output, opposite
        // status - previously indistinguishable.
        poison(out);
        ASSERT_FALSE(KukuTable_Table(t, 64, out));
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        poison(out);
        ASSERT_TRUE(KukuTable_Stash(t, 0, out));
        poison(out);
        ASSERT_FALSE(KukuTable_Stash(t, 2, out));

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, TryLocationReportsStatus)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t item[2] = { 77, 78 };
        uint32_t loc = 0xFFFFFFFFU;
        ASSERT_TRUE(KukuTable_TryLocation(t, item, 0, &loc));
        ASSERT_LT(loc, default_table_size);
        // The legacy accessor must agree with the new one on the success path.
        ASSERT_EQ(KukuTable_Location(t, item, 0), loc);

        // An out-of-range location function index fails, and the out-parameter is defined.
        loc = 0xFFFFFFFFU;
        ASSERT_FALSE(KukuTable_TryLocation(t, item, default_loc_func_count, &loc));
        ASSERT_EQ(0U, loc);

        // The empty item is rejected.
        uint64_t empty[2];
        set_all_ones(empty);
        loc = 0xFFFFFFFFU;
        ASSERT_FALSE(KukuTable_TryLocation(t, empty, 0, &loc));
        ASSERT_EQ(0U, loc);

        // Null arguments.
        loc = 0xFFFFFFFFU;
        ASSERT_FALSE(KukuTable_TryLocation(nullptr, item, 0, &loc));
        ASSERT_EQ(0U, loc);
        ASSERT_FALSE(KukuTable_TryLocation(t, item, 0, nullptr));

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, OutOfRangeTableAndStashReads)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t out[2];
        poison(out);
        ASSERT_FALSE(KukuTable_Table(t, default_table_size, out));
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        poison(out);
        ASSERT_FALSE(KukuTable_Stash(t, default_stash_size, out));
        ASSERT_EQ(0U, out[0]);
        ASSERT_EQ(0U, out[1]);

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, EmptyItemIsRejectedByMutatingEntryPoints)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t empty[2];
        set_all_ones(empty);
        ASSERT_TRUE(KukuTable_IsEmptyItem(t, empty));
        ASSERT_FALSE(KukuTable_Insert(t, empty));

        uint64_t normal[2] = { 5, 6 };
        ASSERT_FALSE(KukuTable_IsEmptyItem(t, normal));

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, DuplicateInsertReturnsFalse)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t item[2] = { 7, 8 };
        ASSERT_TRUE(KukuTable_Insert(t, item));
        ASSERT_FALSE(KukuTable_Insert(t, item));

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, AllLocationsTwoCallProtocol)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t item[2] = { 11, 12 };
        ASSERT_TRUE(KukuTable_Insert(t, item));

        // A buffer that is large enough returns the locations and the true count.
        vector<uint32_t> locations(Kuku_MaxLocFuncCount(), 0xFFFFFFFFU);
        uint32_t count = static_cast<uint32_t>(locations.size());
        KukuTable_AllLocations(t, item, locations.data(), &count);
        ASSERT_GT(count, 0U);
        ASSERT_LE(count, default_loc_func_count);
        for (uint32_t i = 0; i < count; i++)
        {
            ASSERT_LT(locations[i], default_table_size);
        }

        // A buffer that is too small must write nothing and report the required size.
        const uint32_t required = count;
        if (required > 1)
        {
            vector<uint32_t> small(required - 1, 0xAAAAAAAAU);
            uint32_t small_count = static_cast<uint32_t>(small.size());
            KukuTable_AllLocations(t, item, small.data(), &small_count);
            ASSERT_EQ(required, small_count);
            for (uint32_t v : small)
            {
                ASSERT_EQ(0xAAAAAAAAU, v);
            }
        }

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, AccessorsAndClearTable)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        ASSERT_EQ(default_table_size, KukuTable_TableSize(t));
        ASSERT_EQ(default_stash_size, KukuTable_StashSize(t));
        ASSERT_EQ(default_loc_func_count, KukuTable_LocFuncCount(t));
        ASSERT_DOUBLE_EQ(0.0, KukuTable_FillRate(t));

        uint64_t item[2] = { 21, 22 };
        ASSERT_TRUE(KukuTable_Insert(t, item));
        ASSERT_GT(KukuTable_FillRate(t), 0.0);

        KukuTable_ClearTable(t);
        ASSERT_DOUBLE_EQ(0.0, KukuTable_FillRate(t));
        KukuQueryResult q;
        poison(q);
        ASSERT_FALSE(KukuTable_Query(t, item, &q));

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, NullHandleAccessorsReturnDefinedValues)
    {
        // Zero is not a valid table size or location function count for a
        // successfully constructed table, so it is an unambiguous failure indicator
        // for those two. StashSize and FillRate legitimately can be zero.
        ASSERT_EQ(0U, KukuTable_TableSize(nullptr));
        ASSERT_EQ(0U, KukuTable_LocFuncCount(nullptr));
        ASSERT_EQ(0U, KukuTable_StashSize(nullptr));
        ASSERT_DOUBLE_EQ(0.0, KukuTable_FillRate(nullptr));
        ASSERT_FALSE(KukuTable_Insert(nullptr, nullptr));
        ASSERT_FALSE(KukuTable_IsEmptyItem(nullptr, nullptr));

        // Destroy and ClearTable must tolerate a null handle without crashing.
        KukuTable_Destroy(nullptr);
        KukuTable_ClearTable(nullptr);
    }

    TEST(KukuCTests, LocationsAreInRange)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        uint64_t item[2] = { 31, 32 };
        vector<uint32_t> locations(Kuku_MaxLocFuncCount(), 0);
        uint32_t count = static_cast<uint32_t>(locations.size());
        KukuTable_AllLocations(t, item, locations.data(), &count);
        ASSERT_GT(count, 0U);

        for (uint32_t i = 0; i < default_loc_func_count; i++)
        {
            uint32_t loc = KukuTable_Location(t, item, i);
            ASSERT_LT(loc, default_table_size);
        }

        KukuTable_Destroy(t);
    }

    TEST(KukuCTests, IncrementItemWrapsCorrectly)
    {
        uint64_t item[2] = { 0, 0 };
        Kuku_IncrementItem(item);
        ASSERT_EQ(1U, item[0]);
        ASSERT_EQ(0U, item[1]);

        // Carry from the low word into the high word.
        item[0] = ~uint64_t(0);
        item[1] = 0;
        Kuku_IncrementItem(item);
        ASSERT_EQ(0U, item[0]);
        ASSERT_EQ(1U, item[1]);

        // Full wraparound.
        item[0] = ~uint64_t(0);
        item[1] = ~uint64_t(0);
        Kuku_IncrementItem(item);
        ASSERT_EQ(0U, item[0]);
        ASSERT_EQ(0U, item[1]);

        // A null pointer must be ignored rather than dereferenced.
        Kuku_IncrementItem(nullptr);
    }

    TEST(KukuCTests, SetRandomItemProducesDistinctValues)
    {
        // This cannot force an entropy failure, so it only establishes that the
        // export writes and that repeated calls differ. See the note in kuku.cpp
        // about the all-zero fallback taken when the entropy source is unavailable.
        uint64_t a[2];
        uint64_t b[2];
        poison(a);
        poison(b);
        ASSERT_TRUE(Kuku_SetRandomItem(a));
        ASSERT_TRUE(Kuku_SetRandomItem(b));
        ASSERT_FALSE(a[0] == 0xDEADBEEFDEADBEEFULL && a[1] == 0xFEEDFACEFEEDFACEULL);
        ASSERT_FALSE(a[0] == b[0] && a[1] == b[1]);

        ASSERT_FALSE(Kuku_SetRandomItem(nullptr));
    }

    TEST(KukuCTests, StashPaddingIsReportedAsEmptyItem)
    {
        void *t = make_default_table();
        ASSERT_NE(nullptr, t);

        // Indices within stash_size but beyond the number of stashed items return
        // the empty item rather than being treated as out of range.
        uint64_t empty[2];
        uint64_t slot[2];
        KukuTable_EmptyItem(t, empty);
        poison(slot);
        ASSERT_TRUE(KukuTable_Stash(t, 0, slot));
        ASSERT_EQ(empty[0], slot[0]);
        ASSERT_EQ(empty[1], slot[1]);

        KukuTable_Destroy(t);
    }

    // Implemented in c_linkage.c, which is compiled as C. See that file for why this exists:
    // the C ABI header was once C++-only, and no C++ test could have detected it.
    // The compile-time half of this check is the build of c_linkage.c itself; the runtime
    // half below confirms the ABI is callable and links from a C translation unit.
    extern "C" int kuku_c_smoke_from_c(void);

    TEST(KukuCTests, HeaderIsUsableFromC)
    {
        ASSERT_EQ(1, kuku_c_smoke_from_c());
    }
} // namespace kuku_c_tests
