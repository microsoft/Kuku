// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// This header is the C ABI of Kuku and MUST remain parsable by a C compiler. It therefore
// includes only C headers and declares no C++ constructs. In particular it does NOT include
// kuku/kuku.h: that reaches common.h and <algorithm>, which no C translation unit can
// compile, and including it made this header C++-only despite being the C entry point.
// tests/kuku/c/c_linkage.c is compiled as C for the sole purpose of keeping this true.

#include <stddef.h>
#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

// C++ needs extern "C" to suppress name mangling; in C the construct does not exist.
#ifdef __cplusplus
#define KUKU_C_LINKAGE extern "C"
#else
#define KUKU_C_LINKAGE
#endif

#ifdef _MSC_VER

// Check that architecture (platform) is x64. A preprocessor #error rather than a
// static_assert, so the check also fires for a C consumer.
#ifndef _WIN64
#error "Kuku's C ABI requires a 64-bit (x64) architecture"
#endif

// Windows requires every exported symbol to be marked, so the C ABI is decorated with
// dllexport while the library is being built and dllimport while it is being consumed.
// Neither applies to a static build, which needs no decoration at all - decorating a
// static definition with dllimport is an error (C2491).
//
// KUKU_C_IS_SHARED says whether a shared kukuc exists; it is attached to the kukuc target
// as a PUBLIC compile definition when the shared variant is built, so it reaches consumers
// automatically through the exported CMake target. A consumer that links the shared kukuc
// without using its CMake package should define it manually to get dllimport; omitting it
// still links correctly against the import library, just without the direct-call
// optimization dllimport enables.
//
// The *_EXPORTS half distinguishes building from consuming. CMake defines <target>_EXPORTS
// automatically for a shared library target, which is where kukuc_EXPORTS comes from.
#ifdef KUKU_C_IS_SHARED
#if defined(KUKU_C_EXPORTS) || defined(kuku_c_EXPORTS) || defined(kukuc_EXPORTS)
#define KUKU_C_DECOR KUKU_C_LINKAGE __declspec(dllexport)
#else
#define KUKU_C_DECOR KUKU_C_LINKAGE __declspec(dllimport)
#endif
#else
#define KUKU_C_DECOR KUKU_C_LINKAGE
#endif

#define KUKU_C_CALL __cdecl

#else // _MSC_VER

#define KUKU_C_DECOR KUKU_C_LINKAGE
#define KUKU_C_CALL

#endif // _MSC_VER

#define KUKU_C_FUNC(x) KUKU_C_DECOR x KUKU_C_CALL

// The typedef matters for C: without it `KukuQueryResult` is not a type name there, and
// every declaration below that mentions it would fail to compile.
typedef struct KukuQueryResult
{
    bool found;
    bool in_stash;
    uint32_t location;
    uint32_t loc_func_index;
} KukuQueryResult;

KUKU_C_FUNC(void *)
KukuTable_Create(
    uint32_t table_size, uint32_t stash_size, uint32_t loc_func_count, uint64_t *loc_func_seed, uint64_t max_probe,
    uint64_t *empty_item);

KUKU_C_FUNC(void) KukuTable_Destroy(void *kuku_table);

KUKU_C_FUNC(bool) KukuTable_Insert(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(bool) KukuTable_Query(void *kuku_table, uint64_t *item, KukuQueryResult *query_result);

KUKU_C_FUNC(bool) KukuTable_IsEmptyItem(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(void) KukuTable_EmptyItem(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(void) KukuTable_LeftoverItem(void *kuku_table, uint64_t *item);

KUKU_C_FUNC(double) KukuTable_FillRate(void *kuku_table);

KUKU_C_FUNC(uint32_t) KukuTable_LocFuncCount(void *kuku_table);

// Returns true and writes the slot contents through `item` on success. Returns false when
// `index` is out of range or an argument is null, in which case `item` is set to {0, 0}.
// Callers must check the return value: {0, 0} is itself a legal slot value, and is a common
// choice of empty_item, so the output alone cannot distinguish a failed read from a real
// one.
KUKU_C_FUNC(bool) KukuTable_Table(void *kuku_table, uint32_t index, uint64_t *item);

KUKU_C_FUNC(uint32_t) KukuTable_TableSize(void *kuku_table);

// Returns true and writes the stash entry through `item` on success. See KukuTable_Table for
// why the return value must be checked.
KUKU_C_FUNC(bool) KukuTable_Stash(void *kuku_table, uint32_t index, uint64_t *item);

KUKU_C_FUNC(uint32_t) KukuTable_StashSize(void *kuku_table);

// Reports failure as 0, which is indistinguishable from a legitimate location 0. Prefer
// KukuTable_TryLocation in new code, or validate the arguments beforehand: this function
// fails only when an argument is null, `item` is the empty item, or `loc_func_index` is not
// below KukuTable_LocFuncCount().
KUKU_C_FUNC(uint32_t) KukuTable_Location(void *kuku_table, uint64_t *item, uint32_t loc_func_index);

// Status-returning counterpart to KukuTable_Location. Returns true and writes the location
// through `location` on success; returns false and sets `*location` to 0 on failure.
KUKU_C_FUNC(bool)
KukuTable_TryLocation(void *kuku_table, uint64_t *item, uint32_t loc_func_index, uint32_t *location);

KUKU_C_FUNC(void) KukuTable_AllLocations(void *kuku_table, uint64_t *item, uint32_t *locations, uint32_t *count);

KUKU_C_FUNC(void) KukuTable_ClearTable(void *kuku_table);

// Returns true and writes a fresh random 128-bit item through `item` on success. Returns
// false if the platform's entropy source is unavailable, in which case `item` is set to
// {0, 0}.
//
// Callers MUST check the return value. An all-zero item is a legal value, so the output
// alone cannot distinguish success from failure, and this function is the idiomatic way to
// produce a location-function seed: ignoring the result would silently substitute a fixed,
// publicly known seed for one the caller believes is unpredictable.
KUKU_C_FUNC(bool) Kuku_SetRandomItem(uint64_t *item);

KUKU_C_FUNC(void) Kuku_IncrementItem(uint64_t *item);

// These take (void) rather than (): in C an empty parameter list declares an unprototyped
// function that accepts any arguments, which defeats the compiler's ability to check calls
// across the ABI. In C++ the two spellings are identical, so (void) is correct for both.
KUKU_C_FUNC(uint32_t) Kuku_MinTableSize(void);

KUKU_C_FUNC(uint32_t) Kuku_MaxTableSize(void);

KUKU_C_FUNC(uint32_t) Kuku_MinStashSize(void);

KUKU_C_FUNC(uint32_t) Kuku_MaxStashSize(void);

KUKU_C_FUNC(uint32_t) Kuku_MinLocFuncCount(void);

KUKU_C_FUNC(uint32_t) Kuku_MaxLocFuncCount(void);
