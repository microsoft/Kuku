// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "common.h"

#ifdef _WIN32
#include <Windows.h>
#include <bcrypt.h>
#endif

using namespace std;

uint64_t kuku::random_uint64()
{
    uint64_t result;
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    std::random_device rd("/dev/urandom");
    result = (static_cast<std::uint64_t>(rd()) << 32)
        + static_cast<std::uint64_t>(rd());
#elif defined(_WIN32)
    if (!BCRYPT_SUCCESS(BCryptGenRandom(
        NULL,
        reinterpret_cast<unsigned char*>(&result),
        sizeof(result),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG)))
    {
        throw std::runtime_error("BCryptGenRandom failed");
    }
#else
#error "Unsupported target platform"
#endif
    return result;
}
