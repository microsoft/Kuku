// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// This translation unit is deliberately compiled as C rather than C++, and that is the whole
// point of it.
//
// Until this file existed, kuku/c/kuku.h included kuku/kuku.h, which reaches common.h and
// <algorithm>. Kuku's C ABI header could therefore only ever be parsed by a C++ compiler -
// in a library whose entire reason for existing is to be consumed from C and through FFI.
// The existing C ABI tests could not catch it, because they are C++ translation units
// themselves and so never asked the question.
//
// If anyone reintroduces a C++ construct into that header - a C++ include, extern "C"
// without a __cplusplus guard, an untypedef'd struct tag, a static_assert on a kuku:: type -
// THIS FILE STOPS COMPILING. The check is the compilation, not an assertion.
//
// It also calls the ABI at runtime, so the declarations are exercised for callability and
// linkage rather than merely for parsing.

#include "kuku/c/kuku.h"

int kuku_c_smoke_from_c(void);

// Returns 1 if every step succeeded, 0 on the first failure.
int kuku_c_smoke_from_c(void)
{
    uint64_t seed[2];
    uint64_t empty[2];
    uint64_t item[2];
    uint64_t slot[2];
    uint32_t location = 0;
    KukuQueryResult result = { false, false, 0, 0 };
    void *table = NULL;
    int rc = 0;

    seed[0] = 0;
    seed[1] = 0;
    empty[0] = ~(uint64_t)0;
    empty[1] = ~(uint64_t)0;
    item[0] = 5;
    item[1] = 6;

    table = KukuTable_Create(256, 2, 3, seed, 100, empty);
    if (NULL == table)
    {
        return 0;
    }

    if (!KukuTable_Insert(table, item))
    {
        goto done;
    }

    if (!KukuTable_Query(table, item, &result) || !result.found)
    {
        goto done;
    }

    // Status-returning entry points. A reported location must
    // lie inside the table.
    if (!KukuTable_TryLocation(table, item, 0, &location))
    {
        goto done;
    }

    if (location >= KukuTable_TableSize(table))
    {
        goto done;
    }

    if (!KukuTable_Table(table, 0, slot))
    {
        goto done;
    }

    // A slot the ABI reports as empty must equal the configured empty item.
    if (KukuTable_IsEmptyItem(table, slot) && (slot[0] != empty[0] || slot[1] != empty[1]))
    {
        goto done;
    }

    if (Kuku_MinTableSize() > Kuku_MaxTableSize())
    {
        goto done;
    }

    if (Kuku_MinLocFuncCount() > Kuku_MaxLocFuncCount())
    {
        goto done;
    }

    if (Kuku_MinStashSize() > Kuku_MaxStashSize())
    {
        goto done;
    }

    rc = 1;

done:
    KukuTable_Destroy(table);
    return rc;
}
